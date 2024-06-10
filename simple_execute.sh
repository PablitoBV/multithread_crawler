#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <NUMBER_OF_THREADS> <MAX_LINKS_TO_PROCESS>"
    exit 1
fi

# Assign command line arguments to variables
NUMBER_OF_THREADS=$1
MAX_LINKS_TO_PROCESS=$2

# Run make clean, make, and execute the web crawler
make clean
make
./web "$NUMBER_OF_THREADS" "$MAX_LINKS_TO_PROCESS"


