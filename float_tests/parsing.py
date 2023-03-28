import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import pandas as pd
import glob
import os

DEBUG = False
VIOLIN = False

precision_names = {11: "FP16", 24: "FP32", 53: "FP64"}
order = {"FP16": 1, "FP32": 2, "FP64": 3}
results = pd.DataFrame()
for file in glob.glob("./results/*_float_addition_test.csv"):
    precision = int(os.path.basename(file).split("_")[0])
    dataframe = pd.read_csv(file, dtype={'error': float, 'type': str})
    if DEBUG:
        dataframe.dropna()
        for index, error in dataframe.iterrows():
            if isinstance(error["error"], str):
                print(error)
    dataframe["error"] = np.log10(dataframe["error"])
    name = precision
    if precision in precision_names:
        name = precision_names[precision]
    dataframe["precision"] = name
    results = results.append(dataframe, ignore_index=True)
if DEBUG:
    print(results[results["error"] == 0])

# Plot
sns.set_theme(style="whitegrid", font_scale=2)
f, ax = plt.subplots(figsize=(11, 8))
if VIOLIN:
    sns.violinplot(split=True, inner="quart", linewidth=1, x="precision", y="error", hue="type", data=results, order=order)
else:
    sns.boxplot(palette="RdGy", x="precision", y="error", hue="type", data=results, order=order)

# Tweak the visual presentation
plt.xlabel("")
plt.ylabel("Relative error")
handles, labels = ax.get_legend_handles_labels()
ax.legend(handles=handles, labels=labels)
current_values = plt.gca().get_yticks()
plt.gca().set_yticklabels(["$10^{"+f"{x:.0f}"+"}$" for x in current_values])
plt.savefig('test.png', bbox_inches='tight')
plt.show()
