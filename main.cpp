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

const int maxLinksToProcess = 100;

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

void downloadAndExtract(const std::string& url, std::vector<std::string>& links, std::string& html) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            html = readBuffer;
            GumboOutput* output = gumbo_parse(readBuffer.c_str());
            if (output != nullptr) {
                extractLinks(output->root, links);
                gumbo_destroy_output(&kGumboDefaultOptions, output);
            }
        }
    }
}

void threadFunction(ConcurrentQueue<std::string>& queue) {
    while (true) {
        if (processedLinksCount >= maxLinksToProcess) {
            break;
        }

        std::string url = queue.pop();
        if (url.empty())
            break;

        std::vector<std::string> links;
        std::string html;

        downloadAndExtract(url, links, html);

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            // Save the HTML content to html.txt
            /*
            std::ofstream htmlFile("html.txt", std::ios::app);
            if (htmlFile.is_open()) {
                htmlFile << "URL: " << url << "\n";
                htmlFile << html << "\n\n";
                htmlFile.close();
                std::cout << "HTML content has been saved to html.txt" << std::endl;
            } else {
                std::cerr << "Unable to open file html.txt for writing" << std::endl;
            }
            */

            // Save the extracted links to links.txt
            /*
            std::ofstream outFile("links.txt", std::ios::app);
            if (outFile.is_open()) {
                for (const auto& link : links) {
                    outFile << link << std::endl;
                }
                outFile.close();
                std::cout << "Links have been saved to links.txt" << std::endl;
            } else {
                std::cerr << "Unable to open file links.txt for writing" << std::endl;
            }
            */
        }

        processedLinksCount++;

        for (const auto& link : links) {
            if (processedLinksCount >= maxLinksToProcess) {
                break;
            }

            std::lock_guard<std::mutex> lock(outputMutex);
            if (!visitedLinks.contains(Website(link))) {
                visitedLinks.add(Website(link));
                queue.push(link);
            }
        }
    }
}

int main() {
    const int numThreads = 12;
    ConcurrentQueue<std::string> queue;

    std::string initialUrl = "https://balls.com/"; // Replace with the desired initial URL
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

    return 0;
} 
