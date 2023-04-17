import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import itertools

# Load data and constants
dataframe = pd.read_csv("../tests/implementation/results/osu_allreduce.csv")
markers = itertools.cycle(["X", "+", "o", "^"])
marker = next(markers)
colors = itertools.cycle(['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f',
                          '#bcbd22', '#17becf'])
color = next(colors)


# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 16777216]  # 8388608
dataframe["block_size"] *= 4
dataframe["block_size"] = dataframe["block_size"].apply(str)
dataframe["block_size"] += " B"
dataframe = dataframe.groupby(['dtype', 'op', 'mode', 'nranks', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                                    'avg_tput': np.mean,
                                                                                                    'min_lat': np.min,
                                                                                                    'min_tput': np.min,
                                                                                                    'max_lat': np.max,
                                                                                                    'max_tput': np.max})
dataframe["PPN"] = np.where(dataframe["nranks"] < 72, dataframe["nranks"] / 2, 36)
dataframe["avg_tput"] *= dataframe["PPN"] / 12.5
dataframe["min_tput"] *= dataframe["PPN"] / 12.5
dataframe["max_tput"] *= dataframe["PPN"] / 12.5
dataframe["nranks"] = dataframe["nranks"].apply(str)

# Plot the naive and optimized versions
f, ax = plt.subplots(figsize=(16, 4))

# Plot the bars
baseline = dataframe[dataframe["mode"] == "baseline"]
plt.plot(baseline["nranks"], baseline["avg_tput"], color=color, marker=marker, label='baseline', linewidth=1)
plt.fill_between(baseline["nranks"], baseline["min_tput"], baseline["max_tput"], color=color, alpha=0.2)
optimized = dataframe[dataframe["mode"] == "optimized"]
for block_size in optimized["block_size"].unique():
    color = next(colors)
    marker = next(markers)
    local = optimized[optimized["block_size"] == block_size]
    plt.plot(local["nranks"], local["avg_tput"], color, marker=marker, label=block_size, linewidth=1)
    plt.fill_between(local["nranks"], local["min_tput"], local["max_tput"], color=color, alpha=0.2)

# Tweak the visual presentation
ax.legend(loc="lower right")
ax.set_title("16MB message size", x=0.1, y=0.9)
plt.xlabel("Number of ranks")
plt.ylabel("Fraction of the Aries NIC 100Gbit/s throughput")
ax.set_axisbelow(True)
ax.grid(which="both", alpha=0.5)

# Save file
plt.savefig("./figures/throughput_scaling_percentage.pdf",  bbox_inches='tight')
plt.show()
