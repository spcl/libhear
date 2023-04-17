import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import itertools
import numpy as np
plt.rcParams.update({'font.size': 13})

# Load the data
dataframe = pd.read_csv("../tests/implementation/results/single_core_encr_tput_float.csv")
colors = itertools.cycle(['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b'])
color = next(colors)

# Parse data
dataframe = dataframe[~((dataframe["dtype"] == "int") & (dataframe["func"] == "naive"))]
dataframe['dtype op'] = dataframe['dtype'].str.upper() + " " + dataframe['op']
encryption = dataframe[dataframe['mode'] == 'encryption']
decryption = dataframe[dataframe['mode'] == 'decryption']
hatches = ['/', '+', '\\', '..', 'x', '//']
decryption = decryption.groupby(["dtype op", "func"], as_index=False).agg({'tput': [np.mean, np.max, np.min]})
encryption = encryption.groupby(["dtype op", "func"], as_index=False).agg({'tput': [np.mean, np.max, np.min]})

# Plot
# Decryption
f, ax = plt.subplots(1, 1, figsize=(9, 5))
f.tight_layout()
spread = 0.1
ys = [0, 0.1, 0.2, 0.3, 0.5, 0.6, 1.0, 1.1, 1.2, 1.3, 1.5, 1.6]
width = 0.1
latency_errors = np.zeros((2, 12))
values = decryption[decryption["dtype op"] == "INT sum"][["tput", "func"]].to_numpy()
for index, (mean, max, min, name) in enumerate(values):
    ax.barh(ys[index], mean, width, label=name, color=color, alpha=1)
    latency_errors[0, index] = mean - min
    latency_errors[1, index] = max - mean
    color = next(colors)

values = decryption[decryption["dtype op"] == "FLOAT sum"][["tput", "func"]].to_numpy()
for index, (mean, max, min, name) in enumerate(values):
    ax.barh(ys[index+4], mean, width, label=name, color=color, alpha=1)
    latency_errors[0, index+4] = mean - min
    latency_errors[1, index+4] = max - mean
    color = next(colors)
ax.set_yticks([0.2, 0.55], labels=["INT sum", "FLOAT sum"])

# Encryption
values = encryption[encryption["dtype op"] == "INT sum"][["tput", "func"]].to_numpy()
for index, (mean, max, min, name) in enumerate(values):
    ax.barh(ys[index+6], mean, width, label=name, color=color, alpha=1)
    latency_errors[0, index+6] = mean - min
    latency_errors[1, index+6] = max - mean
    color = next(colors)
values = encryption[encryption["dtype op"] == "FLOAT sum"][["tput", "func"]].to_numpy()
for index, (mean, max, min, name) in enumerate(values):
    ax.barh(ys[index+10], mean, width, label=name, color=color, alpha=1)
    latency_errors[0, index+10] = mean - min
    latency_errors[1, index+10] = max - mean
    color = next(colors)
ax.set_yticks([0.2, 0.55, 1.1, 1.55], labels=["INT sum", "FLOAT sum", "INT sum", "FLOAT sum"])
x_coords = [p.get_width() for p in ax.patches]
y_coords = [p.get_y() + 0.5 * p.get_height() for p in ax.patches]
ax.errorbar(x=x_coords, y=y_coords, xerr=latency_errors, fmt="none", capsize=5, c="k")

# Change the labels:
handles, labels = ax.get_legend_handles_labels()
handles = handles[:6]
labels = labels[:6]
print(labels)
for index, (label, handle) in enumerate(zip(labels, handles)):
    if label == "aesni":
        labels[index] = "AES-NI"
    elif label == "aesni_unroll":
        labels[index] = "AES-NI Unroll"
    elif label == "naive":
        labels[index] = "Unvectorized"
    else:
        labels[index] = label.upper()
print(labels)
# Plot patterns and tweak the visual presentation
axis = ax
for i, bar in enumerate(axis.patches):
    hatch = hatches[i % 6]
    bar.set_hatch(hatch)

axis.set_axisbelow(True)
axis.set_xscale('log')
axis.grid(axis="x")
axis.set_xlabel("")
axis.tick_params('y', labelrotation=45)
axis.text(5, 0.4, s="Decryption", size=14)
axis.text(5, 1.4, s="Encryption", size=14)
axis.legend(handles, labels, bbox_to_anchor=(0, 1., 1, 0.2), loc="lower left", ncol=6)
axis.set_xlabel("Throughput [GB/s]")

# Save file
plt.savefig("./figures/throughput_one_core_enc_dec.svg",  bbox_inches='tight')
plt.show()
