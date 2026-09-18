#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
kdviz.py - Desenha o PARTICIONAMENTO DO PLANO da KD-Tree.
Le figs/data/kdtree_*.space e gera figs/kdtree_*_space.png.

Formato (.space):
    BB <xmin> <xmax> <ymin> <ymax>
    PT <x> <y>
    SEG <x1> <y1> <x2> <y2>
"""
import os, glob
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

DATA = "figs/data"
OUT = "figs"
TIT = {"s1": "inicial", "s2": "apos insercao de (60,50)", "s3": "apos remocao de (30,40)"}

def parse(path):
    bb = (0, 100, 0, 100); pts = []; segs = []
    with open(path) as f:
        for line in f:
            t = line.split()
            if not t: continue
            if t[0] == "BB": bb = tuple(float(v) for v in t[1:5])
            elif t[0] == "PT": pts.append((float(t[1]), float(t[2])))
            elif t[0] == "SEG": segs.append(tuple(float(v) for v in t[1:5]))
    return bb, pts, segs

def desenhar(path):
    bb, pts, segs = parse(path)
    xmin, xmax, ymin, ymax = bb
    fig, ax = plt.subplots(figsize=(6.2, 6.0))
    # os cortes de x sao verticais, os de y sao horizontais: cor por orientacao
    for (x1, y1, x2, y2) in segs:
        cor = "#c0392b" if x1 == x2 else "#2471a3"   # vertical(x)=vermelho, horiz(y)=azul
        ax.plot([x1, x2], [y1, y2], color=cor, lw=1.6, alpha=0.8, zorder=1)
    xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
    ax.scatter(xs, ys, s=90, color="#1a5276", zorder=3)
    for (x, y) in pts:
        ax.annotate(f"({int(x)},{int(y)})", (x, y), textcoords="offset points",
                    xytext=(6, 6), fontsize=9, fontweight="bold", color="#154360")
    ax.set_xlim(xmin - 3, xmax + 3); ax.set_ylim(ymin - 3, ymax + 3)
    base = os.path.basename(path).replace(".space", "")
    s = base.split("_")[1]
    ax.set_title(f"KD-Tree — Particionamento do plano ({TIT.get(s, s)})",
                 fontsize=12, fontweight="bold")
    ax.set_xlabel("x"); ax.set_ylabel("y")
    # legenda manual
    ax.plot([], [], color="#c0392b", lw=2, label="corte em x (nivel par)")
    ax.plot([], [], color="#2471a3", lw=2, label="corte em y (nivel impar)")
    ax.legend(loc="upper right", fontsize=8, framealpha=0.9)
    ax.grid(True, alpha=0.15)
    fig.tight_layout()
    out = os.path.join(OUT, base + "_space.png")
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print("gerado", out)

if __name__ == "__main__":
    for p in sorted(glob.glob(os.path.join(DATA, "*.space"))):
        desenhar(p)
