# CSE305 - Web Crawler Project

## Overview

This project is a multi-threaded web crawler. It uses `libcurl` for downloading web content, `gumbo-parser` for parsing HTML, and custom data structures for concurrent link processing.

## Prerequisites

- `libcurl`
- `gumbo-parser`
- A C++11 compatible compiler
- `make`

## Project Structure

- **main.cpp**: Main source file for the web crawler.
- **ConcurrentQueue.h**: Header file for thread-safe queue implementation.
- **StripedHashSet.h**: Header file for thread-safe hash set implementation.
- **Makefile**: Build configuration for the project.
- **run_simple.sh**: Shell script to run main.
- **Graphing**: Raw data and Python graphing code.  
- **Execute**: Additional Shell scripts for testing. 

## Execution

### Building the Project

To compile the project, use the following commands:

$ make clean

$ make

### Running the Web Crawler

To run the web crawler, execute the compiled binary with the required arguments:

$ ./web <NUMBER_OF_THREADS> <MAX_LINKS_TO_PROCESS> <OUTPUT_CSV_FILE>

For example, to run the crawler with 4 threads and process up to 100 links, saving results to crawler_results.csv:

$ ./web 4 100 crawler_results.csv

Or, use a shell script, such as:

$ ./run_simple.sh 

## Notes

- Ensure that `libcurl` and `gumbo-parser` are installed and properly linked during compilation.
- Adjust `maxLinksToProcess` in `main.cpp` if different from the command line argument.
- The initial URL is set to https://en.wikipedia.org/wiki/Main_Page. Modify it if necessary.

## Authors

Etienne Leroy 

Matei Atodiresei

Pablo Bertaud-Velten

