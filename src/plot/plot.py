import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


data = pd.read_csv("barcode.csv", delimiter=",")
for d, dim in data.groupby("homology dimension"):
    plt.figure()

    for l, line in dim.groupby("line index"):
        epsilon = np.array(line["epsilon"])
        epsilon.sort()
        plt.plot(epsilon, l * np.ones(epsilon.shape))
    plt.title(f"H{d}")
plt.show()