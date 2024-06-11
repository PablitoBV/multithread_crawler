#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// Function to be executed by each thread
void threadFunction(int threadId) {
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    const int maxThreads = std::thread::hardware_concurrency(); // Get the maximum number of hardware threads
    std::cout << "Hardware concurrency: " << maxThreads << " threads.\n";

    int numThreads;
    std::cout << "Enter the number of threads to test: ";
    std::cin >> numThreads;

    if (numThreads > maxThreads) {
        std::cout << "Note: You have requested more threads than the hardware supports.\n";
    }

    std::vector<std::thread> threads;
    auto start = std::chrono::high_resolution_clock::now();

    // Create and run the threads
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(threadFunction, i));
    }

    // Join the threads with the main thread
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken with " << numThreads << " threads: " << elapsed.count() << " seconds\n";

    return 0;
}