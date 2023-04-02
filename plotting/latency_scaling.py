import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import itertools

# Load data and constants
dataframe = pd.read_csv("../tests/results/osu_allreduce.csv")
markers = itertools.cycle(["X", "+", "o", "^"])
marker = next(markers)
colors = itertools.cycle(['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f',
                          '#bcbd22', '#17becf'])
color = next(colors)


# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 16777216]  # 8388608
dataframe = dataframe.groupby(['dtype', 'op', 'mode', 'nranks', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                                    'avg_tput': np.mean,
                                                                                                    'min_lat': np.min,
                                                                                                    'min_tput': np.min,
                                                                                                    'max_lat': np.max,
                                                                                                    'max_tput': np.max})

# Plot the naive and optimized versions
f, ax = plt.subplots(figsize=(16, 4))
# bandwidth = 12.5
# plt.axhline(y=bandwidth)
# plt.text(4, bandwidth*0.987, f" Aries bandwidth [100Gbps]")

# Plot the bars
baseline = dataframe[dataframe["mode"] == "baseline"]
plt.plot(baseline["nranks"], baseline["avg_lat"], color=color, marker=marker, label='baseline', linewidth=1)
plt.fill_between(baseline["nranks"], baseline["min_lat"], baseline["max_lat"], color=color, alpha=0.2)
optimized = dataframe[dataframe["mode"] == "optimized"]
for block_size in optimized["block_size"].unique():
    color = next(colors)
    marker = next(markers)
    local = optimized[optimized["block_size"] == block_size]
    plt.plot(local["nranks"], local["avg_lat"], color, marker=marker, label=block_size, linewidth=1)
    plt.fill_between(local["nranks"], local["min_lat"], local["max_lat"], color=color, alpha=0.2)

# Tweak the visual presentation
ax.set_yscale('log')
ax.legend().set_title("")
plt.xlabel("Number of ranks")
plt.ylabel("Latency [us]")
plt.yticks(rotation=45)
ax.set_axisbelow(True)
ax.grid(axis="x")

# Save file
plt.savefig("./figures/latency_scaling.pdf",  bbox_inches='tight')
plt.show()
