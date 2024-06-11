#!/bin/bash

RESULTS_FILE="crawler_results.csv"
TRIALS=5
THREAD_COUNT=24

# Create or clear the results file
echo "Links,AverageExecutionTime" > $RESULTS_FILE

# Loop through different link counts
for MAX_LINKS_TO_PROCESS in $(seq 100 100 1000); do
    echo "Running with $THREAD_COUNT threads and $MAX_LINKS_TO_PROCESS links..."

    total_time=0

    # Run the web crawler 3 times and sum the execution times
    for trial in $(seq 1 $TRIALS); do
        echo "Trial $trial for $MAX_LINKS_TO_PROCESS links..."

        # Capture the execution time from the C++ program output
        execution_time=$(./web "$THREAD_COUNT" "$MAX_LINKS_TO_PROCESS" "temp_results.csv" | grep "Time taken with" | awk '{print $6}')

        # Add the execution time to the total time
        total_time=$(echo "$total_time + $execution_time" | bc)
    done

    # Calculate the average execution time
    average_time=$(echo "scale=2; $total_time / $TRIALS" | bc)

    # Log the average execution time to the CSV file
    echo "$MAX_LINKS_TO_PROCESS,$average_time" >> $RESULTS_FILE

    echo "Completed runs with $MAX_LINKS_TO_PROCESS links, average execution time: $average_time seconds."
done

echo "All tests completed. Results saved in $RESULTS_FILE."