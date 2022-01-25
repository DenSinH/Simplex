import matplotlib.pyplot as plt
import matplotlib.cm as color_map
import numpy as np
import pandas as pd
import math


title = "Torus"

data = pd.read_csv("barcode.csv", delimiter=",")
cmap = color_map.get_cmap("tab10")
plt.figure()

height = 0
max_dim = len(set(data["homology dimension"]))
max_eps = math.sqrt(max(data[data["end"] < float("inf")]["end"])) / 2

for d, dim in data.groupby("homology dimension"):
    color = cmap(d / max_dim)
    for line, row in dim.iterrows():
        if row["end"] <= 0.125:
            continue

        if abs(row["start"] - row["end"]) > 0.1:
            plt.plot((math.sqrt(row["start"]) / 2, min(math.sqrt(row["end"]) / 2, max_eps)), (height, height), color=color)
            height += 1
    plt.plot(np.linspace(0, max_eps, num=20), (height - 0.5) * np.ones(20), linestyle="--", color="black")
    plt.text(0, height - 0.5, f"$H_{d}$", horizontalalignment="left", verticalalignment="top")

plt.title(title)
plt.xlim(0, max_eps)
plt.xlabel(r"$\epsilon$")
plt.yticks([])
plt.savefig(f"{title.lower()}_barcode.png", dpi=400, bbox_inches="tight")
plt.show()
