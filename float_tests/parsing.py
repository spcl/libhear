import seaborn as sns
import matplotlib.pyplot as plt
import math
import numpy as np
import pandas as pd
import glob
import os

plt.rcParams.update({'font.size': 22})

precision_names = {11: "Half precision", 24: "Single precision", 53: "Double precision"}
order = {"Half precision": 1, "Single precision": 2, "Double precision": 3}
results = pd.DataFrame()
for file in glob.glob("./results/*_float_addition.csv"):
    precision = int(os.path.basename(file).split("_")[0])
    dataframe = pd.read_csv(file, header=None, usecols=[0, 1], names=["error", "type"],
                            dtype={'encrypted_error': float, 'type': str})
    dataframe.dropna()
    # dataframe["error"] = (-1)**(dataframe["error"] < 0) * np.log2(abs(dataframe["error"] / 2**(-precision)))
    dataframe["error"] = np.log10(dataframe["error"])
    # dataframe["error"] = dataframe["error"][dataframe["error"] != 0]
    # dataframe.replace(-np.inf, 0, inplace=True)
    name = precision
    if precision in precision_names:
        name = precision_names[precision]
    dataframe["precision"] = name
    results = results.append(dataframe, ignore_index=True)

print(results[results["error"] < 0])
print(results[results["error"] == 0])
sns.violinplot(split=True, inner="quart", linewidth=1, palette="pastel", x="precision", y="error", hue="type",
               data=results, order=order)
# sns.boxplot(palette="RdGy", x="precision", y="error", hue="type",
#                data=results, order=order)
# sns.catplot(x="precision", y="error", hue="type", data=results)
#plt.yscale("function", functions=[log_scale, log_scale_inverse])
# plt.yscale("log")
# plt.yscale("symlog", linthresh=1e-50)
plt.xlabel("")
plt.ylabel("Observed relative errors")
current_values = plt.gca().get_yticks()
plt.gca().set_yticklabels(["$10^{"+f"{x:.0f}"+"}$" for x in current_values])
plt.show()
