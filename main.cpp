#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include <curl/curl.h>
#include <gumbo.h>
#include <mutex>
#include <chrono>

#include "ConcurrentQueue.h"
#include "StripedHashSet.h"

std::mutex outputMutex;
BaseHashSet visitedLinks(100); // Initialize the striped hash set with a capacity
std::atomic<int> processedLinksCount(0);

int maxLinksToProcess = 100;

// Mutexes for timing values
std::mutex downloadExtractMutex;
std::mutex addToVisitedMutex;
std::mutex addToQueueMutex;
std::mutex addTimeMutex;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc &e) {
        return 0;
    }
    return newLength;
}

void extractLinks(const GumboNode* node, std::vector<std::string>& links) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    if (node->v.element.tag == GUMBO_TAG_A) {
        GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href) {
            std::string link = href->value;
            if (link.find("https://") == 0) {
                links.push_back(link);
            }
        }
    }

    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        extractLinks(static_cast<GumboNode*>(children->data[i]), links);
    }
}

void downloadAndExtract(const std::string& url, std::vector<std::string>& links, std::string& html, double& downloadExtractTime) {
    auto start = std::chrono::high_resolution_clock::now();

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Set timeout to 10 seconds
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            html = readBuffer;
            GumboOutput* output = gumbo_parse(readBuffer.c_str());
            if (output != nullptr) {
                extractLinks(output->root, links);
                gumbo_destroy_output(&kGumboDefaultOptions, output);
            }
        } else {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << " for URL: " << url << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    downloadExtractTime = elapsed.count();
}

void threadFunction(ConcurrentQueue<std::string>& queue, double& totalDownloadExtractTime, double& totalAddToVisitedTime, double& totalAddToQueueTime, double& totalAddTime) {
    while (true) {
        if (processedLinksCount >= maxLinksToProcess) {
            break;
        }

        std::string url = queue.pop();
        if (url.empty()) {
            break;
        }

        std::vector<std::string> links;
        std::string html;

        double downloadExtractTime = 0.0;
        downloadAndExtract(url, links, html, downloadExtractTime);

        {
            std::lock_guard<std::mutex> lock(downloadExtractMutex);
            totalDownloadExtractTime += downloadExtractTime;
        }

        auto startAdd = std::chrono::high_resolution_clock::now();
        double addToVisitedTime = 0.0;
        double addToQueueTime = 0.0;

        {
            std::lock_guard<std::mutex> lock(outputMutex);
        }

        processedLinksCount++;

        for (const auto& link : links) {
            if (processedLinksCount >= maxLinksToProcess) {
                break;
            }

            auto startAddToVisited = std::chrono::high_resolution_clock::now();

            std::lock_guard<std::mutex> lock(outputMutex);
            if (!visitedLinks.contains(Website(link)) && !link.empty()) {
                visitedLinks.add(Website(link));

                auto endAddToVisited = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsedAddToVisited = endAddToVisited - startAddToVisited;
                addToVisitedTime += elapsedAddToVisited.count();

                auto startAddToQueue = std::chrono::high_resolution_clock::now();

                queue.push(link);

                auto endAddToQueue = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsedAddToQueue = endAddToQueue - startAddToQueue;
                addToQueueTime += elapsedAddToQueue.count();
            }
        }

        auto endAdd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedAdd = endAdd - startAdd;

        {
            std::lock_guard<std::mutex> lock(addTimeMutex);
            totalAddTime += elapsedAdd.count();
            totalAddToVisitedTime += addToVisitedTime;
            totalAddToQueueTime += addToQueueTime;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <NUMBER_OF_THREADS> <MAX_LINKS_TO_PROCESS> <CSV_FILE>" << std::endl;
        return 1;
    }

    int numThreads = std::stoi(argv[1]);
    maxLinksToProcess = std::stoi(argv[2]);
    std::string csvFile = argv[3];

    std::cout << "Running with " << numThreads << " threads, processing up to " << maxLinksToProcess << " links, saving results to " << csvFile << std::endl;

    ConcurrentQueue<std::string> queue;

    std::string initialUrl = "https://en.wikipedia.org/wiki/Main_Page";
    visitedLinks.add(Website(initialUrl));
    queue.push(initialUrl);

    double totalDownloadExtractTime = 0.0;
    double totalAddToVisitedTime = 0.0;
    double totalAddToQueueTime = 0.0;
    double totalAddTime = 0.0;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(threadFunction, std::ref(queue), std::ref(totalDownloadExtractTime), std::ref(totalAddToVisitedTime), std::ref(totalAddToQueueTime), std::ref(totalAddTime)));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken with " << numThreads << " threads: " << elapsed.count() << " seconds" << std::endl;

    // Save the timing result to the CSV file
    std::ofstream csvFileStream;
    csvFileStream.open(csvFile, std::ios_base::app);
    if (csvFileStream.is_open()) {
        csvFileStream << numThreads << "," << elapsed.count() << "," << totalDownloadExtractTime << "," << totalAddToVisitedTime << "," << totalAddToQueueTime << "," << totalAddTime << std::endl;
        csvFileStream.close();
    } else {
        std::cerr << "Unable to open CSV file for writing." << std::endl;
    }

    return 0;
}
