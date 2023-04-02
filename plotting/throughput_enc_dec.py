import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd

# Load the data
dataframe = pd.read_csv("../tests/results/single_core_encr_tput.csv")

# Parse data
dataframe['dtype op'] = dataframe['dtype'].str.upper() + " " + dataframe['op']
dataframe = dataframe[dataframe["func"] != "naive"]
encryption = dataframe[dataframe['mode'] == 'encryption']
decryption = dataframe[dataframe['mode'] == 'decryption']
hatches = ['/', 'OO', '\\', '..', 'x', 'O', '.']

# Plot
f, ax = plt.subplots(1, 2, figsize=(6, 9))
f.tight_layout()
sns.barplot(data=decryption, x="dtype op", y="tput", hue="func", palette="tab10", ax=ax[0], alpha=1)
sns.barplot(data=encryption, x="dtype op", y="tput", hue="func", palette="tab10", ax=ax[1], alpha=1)

# Change the labels:
handles, labels = ax[0].get_legend_handles_labels()
for index, (label, handle) in enumerate(zip(labels, handles)):
    if label == "aesni":
        labels[index] = "AES-NI"
    else:
        labels[index] = label.upper()

# Plot patterns and tweak the visual presentation
for index, axis in enumerate(ax):
    for i, bar in enumerate(axis.patches):
        hatch = hatches[i]
        bar.set_hatch(hatch)

    axis.set_axisbelow(True)
    axis.set_yscale('log')
    axis.grid()
    axis.set_xlabel("")
    if index == 0:
        axis.set_title("Encryption", x=0.22, y=0.965)
        axis.set_ylabel("Throughput [GB/s]")
        axis.legend(handles, labels, bbox_to_anchor=(0, 1.02, 1, 0.2), loc="lower left",
                    ncol=len(dataframe["func"].unique()))
    else:
        axis.set_title("Decryption", x=0.22, y=0.965)
        axis.set_ylabel("")
        axis.legend().remove()

# Save file
plt.savefig("./figures/throughput_one_core_enc_dec.pdf",  bbox_inches='tight')
plt.show()
