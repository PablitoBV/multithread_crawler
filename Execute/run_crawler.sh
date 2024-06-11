#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <MAX_LINKS_TO_PROCESS>"
    exit 1
fi

MAX_LINKS_TO_PROCESS=$1
RESULTS_FILE="crawler_results.csv"
TRIALS=3

# Create or clear the results file
echo "Threads,AverageExecutionTime" > $RESULTS_FILE

# Loop through different thread counts
for THREAD_COUNT in $(seq 1 26); do
    echo "Running with $THREAD_COUNT threads..."

    total_time=0

    # Run the web crawler 5 times and sum the execution times
    for trial in $(seq 1 $TRIALS); do
        echo "Trial $trial for $THREAD_COUNT threads..."

        # Capture the execution time from the C++ program output
        execution_time=$(./web "$THREAD_COUNT" "$MAX_LINKS_TO_PROCESS" "temp_results.csv" | grep "Time taken with" | awk '{print $6}')

        # Add the execution time to the total time
        total_time=$(echo "$total_time + $execution_time" | bc)
    done

    # Calculate the average execution time
    average_time=$(echo "scale=2; $total_time / $TRIALS" | bc)

    # Log the average execution time to the CSV file
    echo "$THREAD_COUNT,$average_time" >> $RESULTS_FILE

    echo "Completed runs with $THREAD_COUNT threads, average execution time: $average_time seconds."
done

echo "All tests completed. Results saved in $RESULTS_FILE."