import matplotlib.pyplot as plt
import matplotlib.cm as color_map
import numpy as np
import pandas as pd


title = "Torus"

data = pd.read_csv("barcode.csv", delimiter=",")
cmap = color_map.get_cmap("tab10")
plt.figure()

height = 0
max_dim = len(set(data["homology dimension"]))
min_eps = min(data["epsilon"])
max_eps = max(data["epsilon"])

for d, dim in data.groupby("homology dimension"):
    color = cmap(d / max_dim)
    for _, line in dim.groupby("line index"):
        epsilon = np.array(line["epsilon"])
        epsilon.sort()
        plt.plot(epsilon, height * np.ones(epsilon.shape), color=color)
        height += 1
    plt.plot(np.linspace(0, max_eps, num=20), (height - 0.5) * np.ones(20), linestyle="--", color="black")
    plt.text(min_eps, height - 0.5, f"$H_{d}$", horizontalalignment="left", verticalalignment="top")

plt.title(title)
plt.xlim(min_eps, max_eps)
plt.xlabel(r"$\epsilon$")
plt.yticks([])
plt.savefig("barcode.png", dpi=400, bbox_inches="tight")
plt.show()
