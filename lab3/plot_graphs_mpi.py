import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import glob

result_files = glob.glob("experiment_results_mpi_*proc.csv")
if not result_files:
    result_files = glob.glob("experiment_results_mpi.csv")

all_data = {}
for file in result_files:
    df = pd.read_csv(file)
    procs = df['processes'].iloc[0]
    all_data[procs] = df
    print(f"Загружены данные для {procs} процессов из {file}")

if not all_data:
    print("Нет файлов с результатами!")
    exit()

# ГРАФИК 1
fig1, ax1 = plt.subplots(figsize=(12, 7))

colors = plt.cm.viridis(np.linspace(0, 1, len(all_data)))
for (procs, df), color in zip(sorted(all_data.items()), colors):
    ax1.plot(df['N'], df['time_sec'], 'o-', linewidth=2, markersize=8, 
             color=color, label=f'{procs} процесс(ов)')

ax1.set_xlabel('Размер матрицы N', fontsize=12)
ax1.set_ylabel('Время (секунды)', fontsize=12)
ax1.set_title('Зависимость времени выполнения от размера матрицы\n(разное число MPI процессов)', fontsize=14)
ax1.grid(True, alpha=0.3)
ax1.legend()
plt.tight_layout()
plt.savefig('experiment_graph_mpi.png', dpi=150, bbox_inches='tight')
print("График 1 сохранен: experiment_graph_mpi.png")

# ГРАФИК 2
fig2, ax2 = plt.subplots(figsize=(12, 7))

for (procs, df), color in zip(sorted(all_data.items()), colors):
    ax2.plot(df['N'], df['MFLOPS'], 's-', linewidth=2, markersize=8, 
             color=color, label=f'{procs} процесс(ов)')

ax2.set_xlabel('Размер матрицы N', fontsize=12)
ax2.set_ylabel('MFLOPS', fontsize=12)
ax2.set_title('Производительность для разного числа MPI процессов', fontsize=14)
ax2.grid(True, alpha=0.3)
ax2.legend()
plt.tight_layout()
plt.savefig('mflops_graph_mpi.png', dpi=150, bbox_inches='tight')
print("График 2 сохранен: mflops_graph_mpi.png")

# ГРАФИК 3
fig3, ax3 = plt.subplots(figsize=(12, 7))

base_time = all_data[1]['time_sec'].values  # время для 1 процесса

for (procs, df), color in zip(sorted(all_data.items()), colors):
    if procs == 1:
        continue
    speedup = base_time / df['time_sec'].values
    ax3.plot(df['N'], speedup, 'o-', linewidth=2, markersize=8, 
             color=color, label=f'{procs} процесс(ов)')
    # Идеальное ускорение
    ax3.axhline(y=procs, color=color, linestyle='--', alpha=0.3)

ax3.set_xlabel('Размер матрицы N', fontsize=12)
ax3.set_ylabel('Ускорение', fontsize=12)
ax3.set_title('Ускорение относительно 1 процесса', fontsize=14)
ax3.grid(True, alpha=0.3)
ax3.legend()
plt.tight_layout()
plt.savefig('speedup_graph_mpi.png', dpi=150, bbox_inches='tight')
print("График 3 сохранен: speedup_graph_mpi.png")

# ГРАФИК 4
fig4, ax4 = plt.subplots(figsize=(12, 7))

for (procs, df), color in zip(sorted(all_data.items()), colors):
    if procs == 1:
        continue
    efficiency = (base_time / df['time_sec'].values) / procs * 100
    ax4.plot(df['N'], efficiency, 'D-', linewidth=2, markersize=8, 
             color=color, label=f'{procs} процесс(ов)')

ax4.set_xlabel('Размер матрицы N', fontsize=12)
ax4.set_ylabel('Эффективность (%)', fontsize=12)
ax4.set_title('Эффективность распараллеливания', fontsize=14)
ax4.grid(True, alpha=0.3)
ax4.legend()
plt.tight_layout()
plt.savefig('efficiency_graph_mpi.png', dpi=150, bbox_inches='tight')
print("График 4 сохранен: efficiency_graph_mpi.png")

# ГРАФИК 5
fig5, ax5 = plt.subplots(figsize=(12, 7))

for (procs, df), color in zip(sorted(all_data.items()), colors):
    ax5.loglog(df['N'], df['time_sec'], 'o-', linewidth=2, markersize=8, 
               color=color, label=f'{procs} процесс(ов)')

scale_factor = all_data[1]['time_sec'].iloc[0] / (all_data[1]['N'].iloc[0]**3)
n_theor = np.array([200, 400, 800, 1200, 1600, 2000])
ax5.loglog(n_theor, n_theor**3 * scale_factor, 'k--', label='Теоретическая O(N³)', alpha=0.5)

ax5.set_xlabel('Размер матрицы N', fontsize=12)
ax5.set_ylabel('Время (секунды)', fontsize=12)
ax5.set_title('Логарифмический масштаб', fontsize=14)
ax5.grid(True, alpha=0.3, which='both')
ax5.legend()
plt.tight_layout()
plt.savefig('log_graph_mpi.png', dpi=150, bbox_inches='tight')
print("График 5 сохранен: log_graph_mpi.png")

# ГРАФИК 6
fig6, ax6 = plt.subplots(figsize=(12, 7))

for (procs, df), color in zip(sorted(all_data.items()), colors):
    ax6.plot(df['N'], df['time_sec'], 'o-', linewidth=2, markersize=8, 
             color=color, label=f'{procs} процесс(ов)')

ax6.plot(n_theor, n_theor**3 * scale_factor, 'k--', label='O(N³) теоретическая', alpha=0.5, linewidth=2)

ax6.set_xlabel('Размер матрицы N', fontsize=12)
ax6.set_ylabel('Время (секунды)', fontsize=12)
ax6.set_title('Сравнение с теоретической сложностью', fontsize=14)
ax6.grid(True, alpha=0.3)
ax6.legend()
plt.tight_layout()
plt.savefig('complexity_graph_mpi.png', dpi=150, bbox_inches='tight')
print("График 6 сохранен: complexity_graph_mpi.png")

# ВЫВОД
print("\n" + "="*60)
print("СТАТИСТИКА ЭКСПЕРИМЕНТОВ:")
print("="*60)

for procs in sorted(all_data.keys()):
    df = all_data[procs]
    print(f"\n--- {procs} процессов ---")
    print(f"Минимальное время: {df['time_sec'].min():.4f} с (N={df[df['time_sec']==df['time_sec'].min()]['N'].values[0]})")
    print(f"Максимальное время: {df['time_sec'].max():.2f} с (N={df[df['time_sec']==df['time_sec'].max()]['N'].values[0]})")
    print(f"Средняя производительность: {df['MFLOPS'].mean():.2f} MFLOPS")
    print(f"Максимальная производительность: {df['MFLOPS'].max():.2f} MFLOPS")

plt.show()