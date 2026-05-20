import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('Agg')

sizes = [200, 400, 800, 1200, 1600, 2000]
configs = ['16x16', '32x32', '8x8', '16x8', '32x16']
colors = ['blue', 'red', 'green', 'orange', 'purple']
markers = ['o', 's', '^', 'd', 'v']

times_data = {}
mflops_data = {}

for config in configs:
    filename = f"experiment_results_cuda_{config}.txt"
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
        times = []
        mflops = []
        for line in lines:
            parts = line.strip().split()
            if len(parts) >= 3 and parts[0].isdigit():
                times.append(float(parts[1]))
                mflops.append(float(parts[2]))
        if len(times) == len(sizes):
            times_data[config] = times
            mflops_data[config] = mflops
    except:
        pass

fig, axes = plt.subplots(1, 2, figsize=(14, 5))

ax1 = axes[0]
for i, config in enumerate(configs):
    if config in times_data:
        ax1.plot(sizes, times_data[config], color=colors[i], marker=markers[i], linewidth=2, markersize=6, label=config)
ax1.set_xlabel('Размер матрицы (n×n)')
ax1.set_ylabel('Время (секунды)')
ax1.set_title('Время выполнения (CUDA)')
ax1.grid(True, alpha=0.3)
ax1.legend()

ax2 = axes[1]
for i, config in enumerate(configs):
    if config in mflops_data:
        ax2.plot(sizes, mflops_data[config], color=colors[i], marker=markers[i], linewidth=2, markersize=6, label=config)
ax2.set_xlabel('Размер матрицы (n×n)')
ax2.set_ylabel('Производительность (MFLOPS)')
ax2.set_title('Производительность (CUDA)')
ax2.grid(True, alpha=0.3)
ax2.legend()

plt.tight_layout()
plt.savefig('performance_plot_cuda.png', dpi=150)
print("График сохранён как performance_plot_cuda.png")
