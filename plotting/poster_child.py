import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import itertools
plt.rcParams.update({'font.size': 13})

# Load data and constants
floats = pd.read_csv("../tests/implementation/results/osu_allreduce_float.csv")
ints = pd.read_csv("../tests/implementation/results/osu_allreduce.csv")
combined = pd.concat([floats, ints])
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']
hatches = ['/', '.', '\\', '..', 'x', 'O', '.']

# Filter the data
combined["block_size"] *= 4
combined["block_size"] = combined["block_size"].apply(str)
combined["block_size"] += " B"
latency = combined[combined["msgsize"] == 16].groupby(['dtype', 'mode', 'nranks', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                                    'avg_tput': np.mean,
                                                                                                    'min_lat': np.min,
                                                                                                    'min_tput': np.min,
                                                                                                    'max_lat': np.max,
                                                                                                    'max_tput': np.max})
throughput = combined[combined["msgsize"] == 16777216].groupby(['dtype', 'mode', 'nranks', 'block_size'], as_index=False).agg({'avg_lat': np.mean,
                                                                                                    'avg_tput': np.mean,
                                                                                                    'min_lat': np.min,
                                                                                                    'min_tput': np.min,
                                                                                                    'max_lat': np.max,
                                                                                                    'max_tput': np.max})
latency = latency[(latency["nranks"] == 576) | (latency["nranks"] == 1152)]
throughput = throughput[(throughput["nranks"] == 576) | (throughput["nranks"] == 1152)]

grouped_throughput = throughput.groupby(["dtype", "mode", "nranks"], as_index=False)
filtered_throughput = grouped_throughput.apply(lambda x: x[x.avg_tput == x.avg_tput.max()])
grouped_latency = latency.groupby(["dtype", "mode", "nranks"], as_index=False)
filtered_latency = grouped_latency.apply(lambda x: x[x.avg_lat == x.avg_lat.min()])

filtered_throughput = filtered_throughput[["dtype", "mode", "nranks", "avg_tput", "min_tput", "max_tput"]]
for dtype in ["float", "int"]:
    for nranks in [576, 1152]:
        avg = float(filtered_throughput[(filtered_throughput["mode"] == "baseline") & (filtered_throughput["dtype"] == dtype)
                                  & (filtered_throughput["nranks"] == nranks)]["avg_tput"])
        filtered_throughput.loc[(filtered_throughput["dtype"] == dtype) & (filtered_throughput["nranks"] == nranks), "avg_tput"] /= avg
        filtered_throughput.loc[
            (filtered_throughput["dtype"] == dtype) & (filtered_throughput["nranks"] == nranks), "min_tput"] /= avg
        filtered_throughput.loc[
            (filtered_throughput["dtype"] == dtype) & (filtered_throughput["nranks"] == nranks), "max_tput"] /= avg

filtered_latency = filtered_latency[["dtype", "mode", "nranks", "avg_lat", "min_lat", "max_lat"]]
for dtype in ["float", "int"]:
    for nranks in [576, 1152]:
        avg = float(filtered_latency[(filtered_latency["mode"] == "baseline") & (filtered_latency["dtype"] == dtype)
                                  & (filtered_latency["nranks"] == nranks)]["avg_lat"])
        filtered_latency.loc[(filtered_latency["dtype"] == dtype) & (filtered_latency["nranks"] == nranks), "avg_lat"] /= avg
        filtered_latency.loc[
            (filtered_latency["dtype"] == dtype) & (filtered_latency["nranks"] == nranks), "min_lat"] /= avg
        filtered_latency.loc[
            (filtered_latency["dtype"] == dtype) & (filtered_latency["nranks"] == nranks), "max_lat"] /= avg

filtered_latency = filtered_latency[filtered_latency["mode"] != "baseline"]
filtered_latency["min_lat"] -= filtered_latency["avg_lat"]
filtered_latency["max_lat"] -= filtered_latency["avg_lat"]
filtered_throughput = filtered_throughput[filtered_throughput["mode"] != "baseline"]
filtered_throughput["min_tput"] -= filtered_throughput["avg_tput"]
filtered_throughput["max_tput"] -= filtered_throughput["avg_tput"]
latency_err = np.abs(filtered_latency[['min_lat', 'max_lat']].T.to_numpy())
throughput_err = np.abs(filtered_throughput[['min_tput', 'max_tput']].T.to_numpy())

# Plot
f, ax = plt.subplots(2, 1, figsize=(3.5, 9))
f.tight_layout()
sns.barplot(data=filtered_latency, x="nranks", y="avg_lat", hue="dtype", palette="tab10", ax=ax[0], alpha=1)
sns.barplot(data=filtered_throughput, x="nranks", y="avg_tput", hue="dtype", palette="tab10", ax=ax[1], alpha=1)

x_coords = [p.get_x() + 0.5 * p.get_width() for p in ax[0].patches]
y_coords = [p.get_height() for p in ax[0].patches]
ax[0].errorbar(x=x_coords, y=y_coords, yerr=latency_err, fmt="none", capsize=5, c="k")

x_coords = [p.get_x() + 0.5 * p.get_width() for p in ax[1].patches]
y_coords = [p.get_height() for p in ax[1].patches]
ax[1].errorbar(x=x_coords, y=y_coords, yerr=throughput_err, fmt="none", capsize=5, c="k")

# Change the labels:
handles, labels = ax[0].get_legend_handles_labels()
for index, (label, handle) in enumerate(zip(labels, handles)):
    if label == "float":
        labels[index] = "Float"
    if label == "int":
        labels[index] = "Int"

# Plot patterns and tweak the visual presentation
for index, axis in enumerate(ax):
    for i, bar in enumerate(axis.patches):
        hatch = hatches[i // 2]
        bar.set_hatch(hatch)

    axis.set_axisbelow(True)
    axis.grid()
    axis.set_xlabel("")
    axis.tick_params('y', labelrotation=45)
    if index == 0:
        axis.text(x=0.3, y=1.1, s="16B message", size=15)
        axis.get_xaxis().set_visible(False)
        axis.set_ylabel("Fraction of Cray MPICH latency")
        axis.legend(handles, labels, loc="lower center", ncol=2, bbox_to_anchor=(0, 1.00, 1, 0.2))
    else:
        axis.text(x=0.26, y=0.83, s="16MB message", size=15)
        axis.set_xlabel("Number of ranks")
        axis.set_ylabel("Fraction of Cray MPICH throughput")
        axis.legend().remove()

# Save file
plt.savefig("./figures/poster_child.pdf",  bbox_inches='tight')
plt.show()
