import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import itertools

# Load data and constants
dataframe = pd.read_csv("../tests/results/critical_path.csv")
stages = ["malloc", "encrypt", "comm", "decrypt", "free"]
order = ["naive", "optimized", "baseline"]
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']
hatches = itertools.cycle(['/', 'OO', '\\', '..', 'x', 'O', '.'])

# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 8].groupby(['dtype', 'op', 'mode'], as_index=False).mean()

# Add the previous stages and overhead
dataframe["overhead"] = 0
comm = dataframe["comm"]
for index, stage in enumerate(stages):
    dataframe["overhead"] += dataframe[stage]
    if index != 0:
        dataframe[stage] += dataframe[stages[index-1]]
dataframe["overhead"] /= dataframe["comm"]
dataframe["overhead"] -= 1
dataframe["overhead"] *= 100
labels = []
for mode in order:
    labels.append(float(dataframe[dataframe["mode"] == mode]["overhead"]))

# Plot in the reverse order
f, ax = plt.subplots(figsize=(16, 1))
for index, stage in enumerate(stages[-1::-1]):
    sns.barplot(data=dataframe, x=stage, y=dataframe["mode"], color=colors[index], label=stage, order=order, alpha=1)

# Plot patterns and overheads
num_locations = len(order)
for i, bar in enumerate(ax.patches):
    if i % num_locations == 0:
        hatch = next(hatches)
    bar.set_hatch(hatch)
    height = bar.get_height()
    if i < len(order):
        ax.text(bar.get_x() + bar.get_width() + 100, bar.get_y() + bar.get_height() / 2, f"{labels[i % 3]:.1f}%", ha="center", va="center")

# Tweak the visual presentation
plt.xlabel("Latency [cycles]")
plt.ylabel("")
plt.legend()
plt.yticks(rotation=45)
ax.set_axisbelow(True)
ax.grid(axis="x")

# Change the labels
handles, labels = ax.get_legend_handles_labels()
new_handles = [None for _ in range(len(handles))]
new_labels = [None for _ in range(len(labels))]
for index, (label, handle) in enumerate(zip(labels, handles)):
    new_index = stages.index(label)
    if label == "malloc":
        label = "mem_alloc"
    elif label == "free":
        label = "mem_free"
    new_labels[new_index] = label
    new_handles[new_index] = handle
ax.legend(new_handles, new_labels, bbox_to_anchor=(0, 1.02, 1, 0.2), loc="lower left", ncol=len(stages))

# Save file
plt.savefig("./figures/latency_breakdown.pdf",  bbox_inches='tight')
plt.show()
