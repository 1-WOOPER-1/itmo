import pandas as pd
import matplotlib.pyplot as plt

df_standard = pd.read_csv('data.csv')
df_jemalloc = pd.read_csv('data_jemalloc.csv')

plt.figure(figsize=(12, 7))

plt.plot(df_standard['Size_Bytes'] / 1024, df_standard['Avg_Time_NS'], 
         marker='o', linestyle='-', color='blue', label='Standard malloc (glibc)', linewidth=2)

plt.plot(df_jemalloc['Size_Bytes'] / 1024, df_jemalloc['Avg_Time_NS'], 
         marker='s', linestyle='--', color='red', label='jemalloc (Facebook)', linewidth=2)

plt.title('Сравнение производительности аллокаторов: glibc vs jemalloc', fontsize=14)
plt.xlabel('Размер выделяемой памяти (КБ)', fontsize=12)
plt.ylabel('Среднее время выделения (нс)', fontsize=12)

plt.axvline(x=128, color='green', linestyle=':', label='mmap threshold (128 KB)')

plt.grid(True, which="both", ls="-", alpha=0.5)

plt.legend()

plt.savefig('malloc_comparison.png')
print("График сохранен как malloc_comparison.png")
plt.show()