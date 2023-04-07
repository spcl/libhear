from matplotlib import pyplot as plt, patches
import seaborn as sns
import pandas as pd
import numpy as np
import itertools
plt.rcParams.update({'font.size': 14})

# Load data and constants
dataframe = pd.read_csv("../tests/implementation/results/osu_allreduce_float.csv")
markers = itertools.cycle(["X", "^", "o", "+"])
marker = next(markers)
colors = itertools.cycle(['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f',
                          '#bcbd22', '#17becf'])
color = next(colors)


# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 16777216]  # 8388608
# dataframe["block_size"] = dataframe["block_size"].apply(str)
# dataframe["block_size"] += " B"
dataframe = dataframe.groupby(['dtype', 'op', 'mode', 'nranks', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                                    'avg_tput': np.mean,
                                                                                                    'min_lat': np.min,
                                                                                                    'min_tput': np.min,
                                                                                                    'max_lat': np.max,
                                                                                                    'max_tput': np.max})
dataframe["PPN"] = np.where(dataframe["nranks"] < 72, dataframe["nranks"] / 2, 36)
dataframe["avg_tput"] *= dataframe["PPN"]
dataframe["min_tput"] *= dataframe["PPN"]
dataframe["max_tput"] *= dataframe["PPN"]
dataframe["nranks"] = dataframe["nranks"].apply(str)

# Plot the naive and optimized versions
f, ax = plt.subplots(figsize=(12, 4))
bandwidth = 12.5
plt.axhline(y=bandwidth)
plt.text(0, bandwidth*0.94, f" Aries NIC 100Gbit/s = 12.5GB/s")

# Plot the bars
baseline = dataframe[dataframe["mode"] == "baseline"]
plt.plot(baseline["nranks"], baseline["avg_tput"], color=color, marker=marker, label='Cray MPICH', linewidth=1)
plt.fill_between(baseline["nranks"], baseline["min_tput"], baseline["max_tput"], color=color, alpha=0.2)
optimized = dataframe[dataframe["mode"] == "optimized"]
optimized = optimized[optimized["block_size"] == 65536]
optimized["block_size"] = "HEAR"
print(baseline["avg_tput"])
print(optimized["avg_tput"])
for block_size in optimized["block_size"].unique():
    print(block_size)
    color = next(colors)
    marker = next(markers)
    local = optimized[optimized["block_size"] == block_size]
    plt.plot(local["nranks"], local["avg_tput"], color, marker=marker, label=block_size, linewidth=1)
    plt.fill_between(local["nranks"], local["min_tput"], local["max_tput"], color=color, alpha=0.2)

# Plot vertical lines
plt.axvline(x=4, linewidth=3, color="black")
plt.text(3.8, 1.4, f" 2 nodes capacity", rotation=90)
plt.axvline(x=5, linewidth=1, color="black")
plt.text(4.8, 1.4, f" 4 nodes", rotation=90)
plt.axvline(x=6, linewidth=1, color="black")
plt.text(5.8, 1.4, f" 8 nodes", rotation=90)
plt.axvline(x=7, linewidth=1, color="black")
plt.text(6.8, 1.4, f" 16 nodes", rotation=90)
plt.axvline(x=8, linewidth=1, color="black")
plt.text(7.8, 1.4, f" 32 nodes", rotation=90)
plt.text(0.3, 6.6, f"PPN scaling", size=18)
plt.text(6.3, 8.5, f"Node scaling", size=18)
plt.text(0, 10.7, " 16MB message size")

# Plot background
ax.add_patch(patches.Rectangle((4, -1), 5, 14, color="lavender", alpha=0.3))
for patch in ax.patches:
    patch.set_zorder(-1)

# Tweak the visual presentation
ax.legend(loc="upper right")
plt.xlabel("Number of ranks")
plt.ylabel("Throughput per node [GB/s]")
ax.set_axisbelow(True)
ax.grid(which="both", alpha=0.5)

# Save file
plt.savefig("./figures/throughput_scaling.pdf",  bbox_inches='tight')
plt.show()
