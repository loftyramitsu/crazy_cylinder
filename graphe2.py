#!/usr/bin/python3
# -*- coding: utf-8 -*-
import pandas as pd
import matplotlib.pyplot as plt

# Chargement des trois champs
df_ux = pd.read_csv("output/ux.csv")
df_uy = pd.read_csv("output/uy.csv")
df_p  = pd.read_csv("output/p.csv")

ux = df_ux.pivot(index="y", columns="x", values="ux")
uy = df_uy.pivot(index="y", columns="x", values="uy")
p  = df_p .pivot(index="y", columns="x", values="p")

champs = [
    (ux, "RdBu_r", r"Vitesse horizontale $u_x$ (m/s)"),
    (uy, "RdBu_r", r"Vitesse verticale $u_y$ (m/s)"),
    (p,  "seismic", r"Surpression $P-\left<P\right>$ (Pa)"),
]

fig, axes = plt.subplots(1, 3, figsize=(15, 6))

for ax, (data, cmap, title) in zip(axes, champs):
    im = ax.imshow(data, origin="lower", cmap=cmap)
    fig.colorbar(im, ax=ax, shrink=0.6)
    ax.set_title(title)
    ax.set_xlabel("x")
    ax.set_ylabel("y")

plt.tight_layout()
plt.savefig("champs.png", dpi=150)
plt.show()
