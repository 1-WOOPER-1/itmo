import psutil
import time
import matplotlib.pyplot as plt
import os
import datetime


INTERVAL = 0.3
DURATION = 10


def count_processes():
    return len(psutil.pids())


process_counts = []
timestamps = []
start_time = time.time()


curr_time = 0
while curr_time < DURATION:
    count = count_processes()
    with open("file.txt", "a") as f:
        f.write(str(count) + "\n")
    process_counts.append(count)
    timestamps.append(curr_time)
    print(f"Время: {int(curr_time)}с, Процессов: {count}")
    time.sleep(INTERVAL)
    curr_time = time.time() - start_time


plt.figure(figsize=(12, 6))
plt.plot(timestamps, process_counts, marker='o', linestyle='-', color='b', markersize=4)
plt.title('Количество процессов со временем (Fork-бомба)')
plt.xlabel('Время (секунды)')
plt.ylabel('Количество процессов')
plt.grid(True)
plt.axvline(x=0, color='r', linestyle='--', label='Начало (теоретически)')
plt.legend()
plt.tight_layout()


timestamp_str = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
filename = f"graph_{timestamp_str}.png"
plt.savefig(filename)
print(f"График сохранен как {filename}")
plt.show()

