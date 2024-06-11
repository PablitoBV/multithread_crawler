#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <MAX_LINKS_TO_PROCESS>"
    exit 1
fi

MAX_LINKS_TO_PROCESS=$1
RESULTS_FILE="crawler_results.csv"
TRIALS=3
TEMP_RESULTS_FILE="temp_results.csv"

# Create or clear the results file
echo "Threads,AverageExecutionTime,AverageDownloadExtractTime,AverageAddToVisitedTime,AverageAddToQueueTime,AverageAddTime" > $RESULTS_FILE

# Loop through different thread counts
for THREAD_COUNT in $(seq 24); do
    echo "Running with $THREAD_COUNT threads..."

    total_time=0
    total_download_extract_time=0
    total_add_to_visited_time=0
    total_add_to_queue_time=0
    total_add_time=0

    # Run the web crawler 3 times and sum the execution times
    for trial in $(seq 1 $TRIALS); do
        echo "Trial $trial for $THREAD_COUNT threads..."

        # Initialize the temp results file with header (optional)
        echo "Threads,TotalTime,TotalDownloadExtractTime,TotalAddToVisitedTime,TotalAddToQueueTime,TotalAddTime" > $TEMP_RESULTS_FILE

        # Run the web crawler and capture all timing values
        ./web "$THREAD_COUNT" "$MAX_LINKS_TO_PROCESS" "$TEMP_RESULTS_FILE"

        # Read the last line from temp_results.csv
        last_line=$(tail -n 1 $TEMP_RESULTS_FILE)

        # Extract the timings
        execution_time=$(echo $last_line | awk -F',' '{print $2}')
        download_extract_time=$(echo $last_line | awk -F',' '{print $3}')
        add_to_visited_time=$(echo $last_line | awk -F',' '{print $4}')
        add_to_queue_time=$(echo $last_line | awk -F',' '{print $5}')
        add_time=$(echo $last_line | awk -F',' '{print $6}')

        # Add the timings to the total times
        total_time=$(echo "$total_time + $execution_time" | bc)
        total_download_extract_time=$(echo "$total_download_extract_time + $download_extract_time" | bc)
        total_add_to_visited_time=$(echo "$total_add_to_visited_time + $add_to_visited_time" | bc)
        total_add_to_queue_time=$(echo "$total_add_to_queue_time + $add_to_queue_time" | bc)
        total_add_time=$(echo "$total_add_time + $add_time" | bc)
    done

    # Calculate the average times
    average_time=$(echo "scale=2; $total_time / $TRIALS" | bc)
    average_download_extract_time=$(echo "scale=2; $total_download_extract_time / $TRIALS" | bc)
    average_add_to_visited_time=$(echo "scale=2; $total_add_to_visited_time / $TRIALS" | bc)
    average_add_to_queue_time=$(echo "scale=2; $total_add_to_queue_time / $TRIALS" | bc)
    average_add_time=$(echo "scale=2; $total_add_time / $TRIALS" | bc)

    # Log the average times to the CSV file
    echo "$THREAD_COUNT,$average_time,$average_download_extract_time,$average_add_to_visited_time,$average_add_to_queue_time,$average_add_time" >> $RESULTS_FILE

    echo "Completed runs with $THREAD_COUNT threads, average execution time: $average_time seconds."
done

echo "All tests completed. Results saved in $RESULTS_FILE."