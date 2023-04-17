import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import itertools
plt.rcParams.update({'font.size': 22})

# Load data and constants
dataframe = pd.read_csv("../tests/implementation/results/dnn_overheads.csv")
hatches = itertools.cycle(['/', 'OO', '\\', '..', 'x', 'O', '.'])

# Filter the data
print(dataframe)
for model in dataframe["model"].unique():
    dataframe.loc[(dataframe["model"] == model) & (dataframe["mode"] == "hear"), "time_per_iteration"] /= \
        float(dataframe[(dataframe["model"] == model) & (dataframe["mode"] == "baseline")]["time_per_iteration"])
dataframe = dataframe[~(dataframe["mode"] == "baseline")]
print(dataframe)
# Plot the naive and optimized versions
f, ax = plt.subplots(figsize=(16, 2.5))

# Plot the bars
plt.barh(dataframe["model"], dataframe["time_per_iteration"], color="#9dbcd4")

# Add percentages
labels = list(dataframe["time_per_iteration"])
for index, label in enumerate(labels):
    labels[index] = f" {label*100:.1f}%"
ax.bar_label(ax.containers[0], label_type="edge", labels=labels)

# Tweak the visual presentation
plt.xlim(0, 1.47)
plt.xlabel("Relative execution time")
plt.yticks(rotation=45)
ax.set_axisbelow(True)
ax.grid(axis="x")

# Save file
plt.savefig("./figures/dnn_overhead.pdf",  bbox_inches='tight')
plt.show()
