#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <thread>
#include <functional>
#include <atomic>
#include <curl/curl.h>
#include <gumbo.h>
#include <mutex>
#include <chrono>

// Include the concurrent queue implementation here (assuming it's defined elsewhere)
#include "ConcurrentQueue.h"

std::mutex outputMutex;
std::unordered_set<std::string> visitedLinks; // To keep track of visited links
std::atomic<int> processedLinksCount(0); // Atomic counter for processed links

const int maxLinksToProcess = 100; // Maximum number of links to process

// Write callback function to store the downloaded data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb; // Calculate the size of the new data
    try {
        s->append((char*)contents, newLength); // Append the new data to the string
    } catch (std::bad_alloc &e) {
        // Handle memory allocation failure
        return 0;
    }
    return newLength; // Return the number of bytes handled
}

// Function to extract links from HTML content using Gumbo
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

// Function to download a web page and extract links
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

// Function to be run by each thread
void threadFunction(ConcurrentQueue<std::string>& queue) {
    while (true) {
        if (processedLinksCount >= maxLinksToProcess) {
            break; // Stop if the maximum number of links has been processed
        }

        std::string url = queue.pop();
        if (url.empty())
            break; // Signal to exit thread

        std::vector<std::string> links;
        std::string html;

        downloadAndExtract(url, links, html);

        // Synchronize output to avoid concurrent write issues
        {
            std::lock_guard<std::mutex> lock(outputMutex);

            // Save the HTML content to html.txt
            /*std::ofstream htmlFile("html.txt", std::ios::app);
            if (htmlFile.is_open()) {
                htmlFile << "URL: " << url << "\n";
                htmlFile << html << "\n\n";
                htmlFile.close();
                std::cout << "HTML content has been saved to html.txt" << std::endl;
            } else {
                std::cerr << "Unable to open file html.txt for writing" << std::endl;
            }*/

            // Save the extracted links to links.txt
            /*std::ofstream outFile("links.txt", std::ios::app);
            if (outFile.is_open()) {
                for (const auto& link : links) {
                    outFile << link << std::endl;
                }
                outFile.close();
                std::cout << "Links have been saved to links.txt" << std::endl;
            } else {
                std::cerr << "Unable to open file links.txt for writing" << std::endl;
            }*/
        }

        // Increment the processed links counter
        processedLinksCount++;

        // Add new links to the queue
        for (const auto& link : links) {
            if (processedLinksCount >= maxLinksToProcess) {
                break; // Stop adding new links if the maximum has been reached
            }

            std::lock_guard<std::mutex> lock(outputMutex);
            if (visitedLinks.find(link) == visitedLinks.end()) {
                visitedLinks.insert(link);
                queue.push(link);
            }
        }
    }
}

int main() {
    const int numThreads = 12; // You can vary this value to see the impact
    ConcurrentQueue<std::string> queue;

    std::string initialUrl = "https://balls.com/"; // Replace with the desired initial URL
    visitedLinks.insert(initialUrl);
    queue.push(initialUrl);

    auto start = std::chrono::high_resolution_clock::now();

    // Create threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
        threads.push_back(std::thread(threadFunction, std::ref(queue)));

    // Join threads
    for (auto& thread : threads)
        thread.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken with " << numThreads << " threads: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}
