#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <gumbo.h>

#include <climits>
#include <thread>
#include <numeric>
#include <iterator>
#include <optional>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// CurrentQueue Class
#include "ConcurrentQueue.h"

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
template <typename T>
void threadFunction(ConcurrentQueue<T>& queue, std::function<void(T)> process) {
    while (true) {
        T element = queue.pop();
        if (!element)
            break; // Signal to exit thread
        process(element);
    }
}

int main() {
    // Example function to process elements
    auto processElement = [](int element) {
        std::cout << "Processing element: " << element << std::endl;

        // call function to go to url, grab other links and add them to the queue

        std::string url = "https://en.wikipedia.org/wiki/Ball_(disambiguation)"; // Replace with the desired URL
        std::vector<std::string> links;
        std::string html;

        downloadAndExtract(url, links, html);

        // Save the HTML content to html.txt
        std::ofstream htmlFile("html.txt");
        if (htmlFile.is_open()) {
            htmlFile << html;
            htmlFile.close();
            std::cout << "HTML content has been saved to html.txt" << std::endl;
        } else {
            std::cerr << "Unable to open file html.txt for writing" << std::endl;
        }

        // Save the extracted links to links.txt
        std::ofstream outFile("links.txt");
        if (outFile.is_open()) {
            for (const auto& link : links) {
                outFile << link << std::endl;
            }
            outFile.close();
            std::cout << "Links have been saved to links.txt" << std::endl;
        } else {
            std::cerr << "Unable to open file links.txt for writing" << std::endl;
        }
    };

    const int numThreads = 5;

    // Create a queue and push some elements
    ConcurrentQueue<int> queue;
    for (int i = 0; i < 10; ++i)
        queue.push(i);

    // Create threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
        threads.push_back(std::thread(threadFunction<int>, std::ref(queue), processElement));

    // Join threads
    for (auto& thread : threads)
        thread.join();

    return 0;
}