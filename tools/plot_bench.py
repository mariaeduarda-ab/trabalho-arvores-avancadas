#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
plot_bench.py - Le os CSVs de bench_out/ e gera os graficos dos experimentos.
Saidas em figs/: g_str.png, g_int.png, g_locality.png, g_kd.png
"""
import os, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = "figs"

def ler(path):
    with open(path) as f:
        r = csv.reader(f)
        head = next(r)
        cols = {h: [] for h in head}
        for row in r:
            for h, v in zip(head, row):
                cols[h].append(float(v))
    return cols

def grafico(cols, series, xcol, titulo, xlabel, ylabel, out, xpct=False):
    fig, ax = plt.subplots(figsize=(7.2, 4.6))
    x = cols[xcol]
    if xpct:
        x = [v * 100 for v in x]
    for chave, (rotulo, cor, marca) in series.items():
        ax.plot(x, cols[chave], marker=marca, color=cor, lw=2, ms=6, label=rotulo)
    ax.set_title(titulo, fontsize=12, fontweight="bold")
    ax.set_xlabel(xlabel); ax.set_ylabel(ylabel)
    ax.grid(True, alpha=0.25)
    ax.legend(fontsize=9)
    fig.tight_layout()
    fig.savefig(os.path.join(OUT, out), dpi=150, bbox_inches="tight")
    plt.close(fig)
    print("gerado", out)

if __name__ == "__main__":
    # 1) Trie x Patricia (strings)
    c = ler("bench_out/bench_str.csv")
    grafico(c, {
        "trie_ins": ("Trie - insercao", "#c0392b", "o"),
        "pat_ins":  ("Patricia - insercao", "#e67e22", "s"),
        "trie_bus": ("Trie - busca", "#2471a3", "^"),
        "pat_bus":  ("Patricia - busca", "#27ae60", "D"),
    }, "n", "Trie x Patricia: tempo total de insercao e busca (strings)",
       "numero de chaves (n)", "tempo (ms)", "g_str.png")

    # 2) Splay x Treap x std::set (inteiros, acesso aleatorio)
    c = ler("bench_out/bench_int.csv")
    grafico(c, {
        "splay_bus": ("Splay - busca", "#c0392b", "o"),
        "treap_bus": ("Treap - busca", "#27ae60", "s"),
        "set_bus":   ("std::set (BST bal.) - busca", "#2471a3", "^"),
    }, "n", "Busca aleatoria: Splay x Treap x BST balanceada (std::set)",
       "numero de chaves (n)", "tempo total de n buscas (ms)", "g_int.png")

    # 3) Localidade de acesso (vantagem da Splay)
    c = ler("bench_out/bench_locality.csv")
    grafico(c, {
        "splay_ms": ("Splay", "#c0392b", "o"),
        "treap_ms": ("Treap", "#27ae60", "s"),
        "set_ms":   ("std::set (BST bal.)", "#2471a3", "^"),
    }, "p", "Efeito da LOCALIDADE de acesso (500 mil consultas, n=50000)",
       "% das consultas concentradas no hot set (100 chaves)",
       "tempo total (ms)", "g_locality.png", xpct=True)

    # 4) KD-Tree x busca linear (vizinho mais proximo)
    c = ler("bench_out/bench_kd.csv")
    grafico(c, {
        "kd_ms":     ("KD-Tree (com poda)", "#27ae60", "o"),
        "linear_ms": ("Busca linear (forca bruta)", "#c0392b", "s"),
    }, "n", "Vizinho mais proximo: KD-Tree x busca linear (2000 consultas)",
       "numero de pontos (n)", "tempo total das consultas (ms)", "g_kd.png")
