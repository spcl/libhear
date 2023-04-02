import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import itertools

# Load data and constants
dataframe = pd.read_csv("../tests/results/block_size.csv")
hatches = itertools.cycle(['/', 'OO', '\\', '..', 'x', 'O', '.'])

# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 16777216]  # 8388608
dataframe = dataframe.groupby(['dtype', 'op', 'mode', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                          'avg_tput': np.mean,
                                                                                          'min_lat': np.min,
                                                                                          'min_tput': np.min,
                                                                                          'max_lat': np.max,
                                                                                          'max_tput': np.max})
dataframe["block_size"] = dataframe["block_size"].apply(str)
dataframe["scaled_tput"] = dataframe["avg_tput"] / float(dataframe[dataframe["mode"] == "baseline"]["avg_tput"])

# Plot the naive and optimized versions
f, ax = plt.subplots(figsize=(16, 4))
bandwidth = 0.347222222
plt.axvline(x=bandwidth)
plt.text(bandwidth*0.987, 4, f" Aries bandwidth [100Gbps]", rotation=90)

# Plot the bars
optimized = dataframe[dataframe["mode"] == "optimized"]
baseline = dataframe[dataframe["mode"] == "baseline"]
mpool_only = dataframe[dataframe["mode"] == "mpool_only"]
plt.barh("baseline", float(baseline["avg_tput"]), color="#2c89a0")
plt.barh("mpool_only", float(mpool_only["avg_tput"]), color="#d35f5f")
plt.barh(optimized["block_size"].unique(), optimized["avg_tput"], color="#9dbcd4")

# Add percentages
ax.bar_label(ax.containers[1], label_type="edge", labels=[f" {float(mpool_only['scaled_tput'])*100:.1f}%"])
labels = list(optimized["scaled_tput"])
for index, label in enumerate(labels):
    labels[index] = f" {label*100:.1f}%"
ax.bar_label(ax.containers[2], label_type="edge", labels=labels)

# Tweak the visual presentation
plt.xlabel("Throughput per rank [GB/s]")
plt.ylabel("Pipelining block size [B]", labelpad=-10)
plt.yticks(rotation=45)
ax.set_axisbelow(True)
ax.grid(axis="x")

# Save file
plt.savefig("./figures/block_size.pdf",  bbox_inches='tight')
plt.show()
