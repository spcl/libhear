import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import itertools

# Load data and constants
dataframe = pd.read_csv("../tests/implementation/results/critical_path.csv")
stages = ["malloc", "encrypt", "comm", "decrypt", "free"]
# order = ["OpenSSL SHA1", "AES-NI + SSE2", "Cray MPICH"]
order = ["AES-NI + SSE2", "Cray MPICH"]
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']
hatches = itertools.cycle(['/', 'OO', '\\', '..', 'x', 'O', '.'])

# Filter the data
dataframe = dataframe[dataframe["msgsize"] == 16].groupby(['dtype', 'op', 'mode'], as_index=False).mean()

# Rename the data
dataframe.loc[dataframe["mode"] == "baseline", "mode"] = "Cray MPICH"
dataframe = dataframe.drop(dataframe.loc[dataframe["mode"] == "naive", "mode"].index)
dataframe.loc[dataframe["mode"] == "optimized", "mode"] = "AES-NI + SSE2"
# dataframe.loc[dataframe["mode"] == "naive", "mode"] = "OpenSSL SHA1"

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
    if mode == "Cray MPICH":
        labels.append("")
    else:
        labels.append(f' {float(dataframe[dataframe["mode"] == mode]["overhead"]):.1f}%')

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
ax.bar_label(ax.containers[0], label_type="edge", labels=labels)

# Tweak the visual presentation
plt.xlabel("Latency [cycles]")
plt.ylabel("")
plt.legend()
plt.yticks(rotation=45)
ax.set_axisbelow(True)
ax.grid(axis="x")

# Change the labels
ax.set_title("16B message size", x=0.93, y=0)
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
