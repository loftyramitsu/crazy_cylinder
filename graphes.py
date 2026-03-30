#!/usr/bin/python3
#-*- coding : utf-8 -*-
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("output/p.csv")
U = df.pivot(index="y", columns="x", values="p")

plt.figure(figsize=(4, 12))
plt.imshow(U, origin="lower", cmap="RdBu_r")
plt.colorbar(label="uy (m/s)")
plt.title("Vitesse verticale")
plt.tight_layout()
#plt.savefig("uy.png", dpi=150)
plt.show()
