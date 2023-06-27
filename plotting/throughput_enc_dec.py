import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
plt.rcParams.update({'font.size': 13})

# Load the data
dataframe = pd.read_csv("../tests/implementation/results/single_core_encr_tput.csv")

# Parse data
dataframe = dataframe[~((dataframe["dtype"] == "int") & (dataframe["func"] == "naive"))]
dataframe['dtype op'] = dataframe['dtype'].str.upper() + " " + dataframe['op']
encryption = dataframe[dataframe['mode'] == 'encryption']
decryption = dataframe[dataframe['mode'] == 'decryption']
hatches = ['/', '.', '\\', '..', 'x', '//']
decryption = decryption.groupby(["dtype op", "func"], as_index=False).agg({'tput': np.mean})
encryption = encryption.groupby(["dtype op", "func"], as_index=False).agg({'tput': np.mean})

# Plot
# Decryption
f, ax = plt.subplots(2, 1, figsize=(9, 3))
f.tight_layout()
ys = [0, 0.1, 0.2, 0.3, 0.5, 0.6]
width = 0.1
values = decryption[decryption["dtype op"] == "INT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    print(ys[index], value, width, name)
    ax[1].barh(ys[index], value, width, label=name)

values = decryption[decryption["dtype op"] == "FLOAT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax[1].barh(ys[index+4], value, width, label=name)
ax[1].set_yticks([0.2, 0.55], labels=["INT sum", "FLOAT sum"])

# Encryption
values = encryption[encryption["dtype op"] == "INT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax[0].barh(ys[index], value, width, label=name)

values = encryption[encryption["dtype op"] == "FLOAT sum"][["tput", "func"]].to_numpy()
for index, (value, name) in enumerate(values):
    ax[0].barh(ys[index+4], value, width, label=name)
ax[0].set_yticks([0.2, 0.55], labels=["INT sum", "FLOAT sum"])

# Change the labels:
handles, labels = ax[0].get_legend_handles_labels()
for index, (label, handle) in enumerate(zip(labels, handles)):
    if label == "aesni":
        labels[index] = "AES-NI"
    else:
        labels[index] = label.upper()

# Plot patterns and tweak the visual presentation
plt.subplots_adjust(hspace=0.4)
for index, axis in enumerate(ax):
    for i, bar in enumerate(axis.patches):
        hatch = hatches[i]
        bar.set_hatch(hatch)

    axis.set_axisbelow(True)
    axis.set_xscale('log')
    axis.grid(axis="x")
    axis.set_xlabel("")
    axis.tick_params('y', labelrotation=45)
    if index == 0:
        axis.set_title("Encryption", x=0.8, y=0.6)
        axis.set_ylabel("")
        axis.legend(handles, labels, bbox_to_anchor=(0, 1.02, 1, 0.2), loc="lower left",
                    ncol=len(dataframe["func"].unique()))
    else:
        axis.set_title("Decryption", x=0.8, y=0.6)
        axis.set_xlabel("Throughput [GB/s]")
        axis.set_ylabel("")
        axis.legend().remove()

# Save file
plt.savefig("./figures/throughput_one_core_enc_dec.pdf",  bbox_inches='tight')
plt.show()
