import pandas as pd
import matplotlib.pyplot as plt

# Load the data
data = pd.read_csv('crawler_results_links.csv')

# Create the plot
plt.figure(figsize=(10, 6))
plt.plot(data['Links'], data['AverageExecutionTime'], marker='o', linestyle='-', color='b')

# Title and labels
plt.title('Web Crawler Performance with 24 Threads')
plt.xlabel('Number of Links')
plt.ylabel('Average Execution Time (seconds)')

# Grid for better readability
plt.grid(True)

# Save the plot as a file
plt.savefig('performance_over_thread_count.png')

# Show the plot
plt.show()
