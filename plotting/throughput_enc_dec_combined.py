import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import itertools
import numpy as np

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
decryption = decryption.groupby(["dtype op", "func"], as_index=False).agg({'tput': np.mean})
encryption = encryption.groupby(["dtype op", "func"], as_index=False).agg({'tput': np.mean})

# Plot
# Decryption
f, ax = plt.subplots(1, 1, figsize=(9, 5))
f.tight_layout()
spread = 0.1
ys = [0, 0.1, 0.2, 0.3, 0.5, 0.6, 1.0, 1.1, 1.2, 1.3, 1.5, 1.6]
width = 0.1
values = decryption[decryption["dtype op"] == "INT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax.barh(ys[index], value, width, label=name, color=color, alpha=1)
    color = next(colors)

values = decryption[decryption["dtype op"] == "FLOAT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax.barh(ys[index+4], value, width, label=name, color=color, alpha=1)
    color = next(colors)
ax.set_yticks([0.2, 0.55], labels=["INT sum", "FLOAT sum"])

# Encryption
values = encryption[encryption["dtype op"] == "INT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax.barh(ys[index+6], value, width, label=name, color=color, alpha=1)
    color = next(colors)
values = encryption[encryption["dtype op"] == "FLOAT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax.barh(ys[index+10], value, width, label=name, color=color, alpha=1)
    color = next(colors)
ax.set_yticks([0.2, 0.55, 1.1, 1.55], labels=["INT sum", "FLOAT sum", "INT sum", "FLOAT sum"])

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

axis.set_xlabel("Throughput [GB/s]")

# Save file
plt.savefig("./figures/throughput_one_core_enc_dec.svg",  bbox_inches='tight')
plt.show()
