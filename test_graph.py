import matplotlib.pyplot as plt

# Data
thread_counts = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 20, 26, 30]
times = [
    (77.9711 + 82.4579 + 74.6788) / 3,
    (36.2507 + 44.1183 + 36.6512) / 3,
    (26.2699 + 25.924 + 24.2386) / 3,
    (18.7436 + 19.3316 + 20.266) / 3,
    (16.1017 + 16.5613 + 15.5087) / 3,
    (13.1539 + 13.8555 + 13.9808) / 3,
    (12.235 + 11.4089 + 11.8179) / 3,
    (11.4152 + 11.5848 + 10.5358) / 3,
    (9.41789 + 9.48107 + 9.35198) / 3,
    (9.24799 + 8.71289 + 8.5526) / 3,
    (8.40758 + 8.75149 + 8.63837) / 3,
    (7.9138 + 8.39428) / 2,  # Assuming the third value is missing
    (5.83298 + 7.07914 + 8.91058) / 3,
    8.20441,
    8.10531
]

# Create the plot
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, times, marker='o', linestyle='-', color='b', label='Time taken')

# Adding titles and labels
plt.title('Effect of Thread Count on Web Crawling Performance')
plt.xlabel('Number of Threads')
plt.ylabel('Time Taken (seconds)')
plt.legend()
plt.grid(True)

# Show the plot
plt.show()
