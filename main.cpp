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
            if (link.find("https://") == 0 && link.find("wikipedia") != std::string::npos) {
                links.push_back(link);
                std::cout << "Extracted link: " << link << std::endl; // Debug print
            }
        }
    }

    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        extractLinks(static_cast<GumboNode*>(children->data[i]), links);
    }
}

void downloadAndExtract(const std::string& url, std::vector<std::string>& links, std::string& html) {
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
}

void threadFunction(ConcurrentQueue<std::string>& queue) {
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

        std::cout << "Processing URL: " << url << std::endl; // Debug print

        downloadAndExtract(url, links, html);

        {
            std::lock_guard<std::mutex> lock(outputMutex);
        }

        processedLinksCount++;
        std::cout << "Processed links count: " << processedLinksCount.load() << std::endl; // Debug print

        for (const auto& link : links) {
            if (processedLinksCount >= maxLinksToProcess) {
                break;
            }

            std::lock_guard<std::mutex> lock(outputMutex);
            if (!visitedLinks.contains(Website(link)) && !link.empty()) {
                visitedLinks.add(Website(link));
                queue.push(link);
                std::cout << "Added to queue: " << link << std::endl; // Debug print
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <NUMBER_OF_THREADS> <MAX_LINKS_TO_PROCESS>" << std::endl;
        return 1;
    }

    int numThreads = std::stoi(argv[1]);
    maxLinksToProcess = std::stoi(argv[2]);

    ConcurrentQueue<std::string> queue;

    std::string initialUrl = "https://en.wikipedia.org/wiki/Main_Page"; // Replace with the desired initial URL
    visitedLinks.add(Website(initialUrl));
    queue.push(initialUrl);

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
        threads.push_back(std::thread(threadFunction, std::ref(queue)));

    for (auto& thread : threads)
        thread.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken with " << numThreads << " threads: " << elapsed.count() << " seconds" << std::endl;

    // Print all websites stored in the StripedHashSet
    //visitedLinks.printAllWebsites();

    return 0;
}
