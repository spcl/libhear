from matplotlib import pyplot as plt, patches
import seaborn as sns
import pandas as pd
import numpy as np
import itertools
plt.rcParams.update({'font.size': 14})

# Load data and constants
dataframe = pd.read_csv("../tests/implementation/results/osu_allreduce.csv")
markers = itertools.cycle(["X", "+", "o", "^"])
marker = next(markers)
colors = itertools.cycle(['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f',
                          '#bcbd22', '#17becf'])
color = next(colors)


# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 16]  # 8388608
dataframe["block_size"] *= 4
dataframe["block_size"] = dataframe["block_size"].apply(str)
dataframe["block_size"] += " B"
dataframe = dataframe.groupby(['dtype', 'op', 'mode', 'nranks', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                                    'avg_tput': np.mean,
                                                                                                    'min_lat': np.min,
                                                                                                    'min_tput': np.min,
                                                                                                    'max_lat': np.max,
                                                                                                    'max_tput': np.max})
dataframe["nranks"] = dataframe["nranks"].apply(str)

# Plot the naive and optimized versions
f, ax = plt.subplots(figsize=(12, 4))

# Plot the bars
baseline = dataframe[dataframe["mode"] == "baseline"]
plt.plot(baseline["nranks"], baseline["avg_lat"], color=color, marker=marker, label='Cray MPICH', linewidth=1)
plt.fill_between(baseline["nranks"], baseline["min_lat"], baseline["max_lat"], color=color, alpha=0.2)
optimized = dataframe[dataframe["mode"] == "optimized"]
optimized["block_size"] = "HEAR"
for block_size in optimized["block_size"].unique():
    color = next(colors)
    marker = next(markers)
    local = optimized[optimized["block_size"] == block_size]
    plt.plot(local["nranks"], local["avg_lat"], color, marker=marker, label=block_size, linewidth=1)
    plt.fill_between(local["nranks"], local["min_lat"], local["max_lat"], color=color, alpha=0.2)

# Plot vertical lines
plt.axvline(x=4, linewidth=3, color="black")
plt.text(3.8, 8, f" 2 nodes capacity", rotation=90)
plt.axvline(x=5, linewidth=1, color="black")
plt.text(4.8, 0.1, f" 4 nodes", rotation=90)
plt.axvline(x=6, linewidth=1, color="black")
plt.text(5.8, 0.1, f" 8 nodes", rotation=90)
plt.axvline(x=7, linewidth=1, color="black")
plt.text(6.8, 0.1, f" 16 nodes", rotation=90)
plt.axvline(x=8, linewidth=1, color="black")
plt.text(7.8, 0.1, f" 32 nodes", rotation=90)
plt.text(1.1, 20, f"PPN scaling", size=18, zorder=5)
plt.text(4.8, 32, f"Node scaling", size=18, zorder=5)
plt.text(2, 37, " 16B message size")

# Plot background
ax.add_patch(patches.Rectangle((4, -1), 5, 60, color="lavender", alpha=0.3))
for patch in ax.patches:
    patch.set_zorder(-1)

# Tweak the visual presentation
ax.legend().set_title("")
plt.xlabel("Number of ranks")
plt.ylabel("Latency [us]")
ax.set_axisbelow(True)
ax.grid(which="both")
plt.yscale("log")

# Save file
plt.savefig("./figures/latency_scaling.pdf",  bbox_inches='tight')
plt.show()
