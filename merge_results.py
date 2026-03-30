import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

#Список потоков (для 2 ядер/4 потока)
threads = [1, 2, 4]
results = {}

print("Чтение результатов")
for t in threads:
    try:
        df = pd.read_csv(f"results_{t}.csv")
        results[t] = df
        print(f"  results_{t}.csv - загружен")
    except FileNotFoundError:
        print(f"  results_{t}.csv - не найден")
        exit(1)

#Создание сводных таблиц
print("\n" + "="*60)
print("Сводная таблица: Время выполнения (секунды)")
print("="*60)

time_table = pd.DataFrame()
mflops_table = pd.DataFrame()

for t in threads:
    df = results[t]
    time_table[f"{t} поток"] = df["time_sec"].values
    mflops_table[f"{t} поток"] = df["MFLOPS"].values

time_table.index = results[threads[0]]["N"].values
mflops_table.index = results[threads[0]]["N"].values

print("\nВремя (сек):")
print(time_table.to_string())

print("\nПроизводительность (MFLOPS):")
print(mflops_table.to_string())

#Расчёт ускорения
print("\n" + "="*60)
print("Ускорение (Speedup = T1 / Tp)")
print("="*60)

speedup = pd.DataFrame()
for t in threads[1:]:
    speedup[f"{t} потоков"] = time_table["1 поток"] / time_table[f"{t} поток"]

speedup.index = results[threads[0]]["N"].values
print(speedup.to_string())

print("\n" + "="*60)
print("Эффективность (Speedup / кол-во потоков)")
print("="*60)

efficiency = pd.DataFrame()
for t in threads[1:]:
    efficiency[f"{t} потоков"] = speedup[f"{t} потоков"] / t

efficiency.index = results[threads[0]]["N"].values
print(efficiency.to_string())

time_table.to_csv("summary_time.csv")
mflops_table.to_csv("summary_mflops.csv")
speedup.to_csv("summary_speedup.csv")
efficiency.to_csv("summary_efficiency.csv")
print("\nФайлы сохранены: summary_time.csv, summary_mflops.csv, summary_speedup.csv, summary_efficiency.csv")

# Построение графиков
fig, axes = plt.subplots(2, 2, figsize=(14, 10))

#График 1: Время выполнения
for t in threads:
    axes[0, 0].plot(results[t]["N"], results[t]["time_sec"], 'o-', linewidth=2, markersize=8, label=f"{t} потоков")
axes[0, 0].set_xlabel("Размер матрицы N")
axes[0, 0].set_ylabel("Время (секунды)")
axes[0, 0].set_title("Зависимость времени выполнения от размера матрицы")
axes[0, 0].legend()
axes[0, 0].grid(True, alpha=0.3)

#График 2: Производительность MFLOPS
for t in threads:
    axes[0, 1].plot(results[t]["N"], results[t]["MFLOPS"], 's-', linewidth=2, markersize=8, label=f"{t} потоков")
axes[0, 1].set_xlabel("Размер матрицы N")
axes[0, 1].set_ylabel("MFLOPS")
axes[0, 1].set_title("Производительность (MFLOPS)")
axes[0, 1].legend()
axes[0, 1].grid(True, alpha=0.3)

#График 3: Ускорение
for t in threads[1:]:
    axes[1, 0].plot(speedup.index, speedup[f"{t} потоков"], '^-', linewidth=2, markersize=8, label=f"{t} потоков")
#Идеальное ускорение для 2 ядер = 2x
axes[1, 0].axhline(y=2, color='r', linestyle='--', linewidth=1.5, label="Идеальное (2x - физические ядра)")
axes[1, 0].set_xlabel("Размер матрицы N")
axes[1, 0].set_ylabel("Ускорение")
axes[1, 0].set_title("Масштабируемость (Speedup)")
axes[1, 0].legend()
axes[1, 0].grid(True, alpha=0.3)

#График 4: Эффективность
for t in threads[1:]:
    axes[1, 1].plot(efficiency.index, efficiency[f"{t} потоков"], 'D-', linewidth=2, markersize=8, label=f"{t} потоков")
axes[1, 1].axhline(y=1.0, color='r', linestyle='--', linewidth=1.5, label="Идеальная (100%)")
axes[1, 1].set_xlabel("Размер матрицы N")
axes[1, 1].set_ylabel("Эффективность")
axes[1, 1].set_title("Эффективность параллелизации")
axes[1, 1].legend()
axes[1, 1].grid(True, alpha=0.3)

plt.suptitle("Лабораторная работа №2: Исследование масштабируемости OpenMP\n(2 ядра / 4 потока)", fontsize=16)
plt.tight_layout()
plt.savefig("openmp_scalability.png", dpi=150, bbox_inches='tight')
print("\nГрафик сохранён: openmp_scalability.png")

plt.show()