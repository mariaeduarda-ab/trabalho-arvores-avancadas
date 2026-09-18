#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
treeviz.py - Le os arquivos figs/data/*.tree e desenha cada arvore como PNG.

Formato de entrada (.tree):
    N <id> <rotulo...>          -> um no
    E <pai> <filho> <rotulo...> -> uma aresta (rotulo pode ser 'L'/'R' ou substring)

Layout: leaves recebem posicoes x consecutivas; no interno fica centralizado
sobre os filhos. Arvores binarias (arestas 'L'/'R') reservam espaco fantasma
para posicionar corretamente filhos unicos a esquerda/direita.
"""
import os, glob
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

DATA = "figs/data"
OUT = "figs"

# titulos amigaveis por estrutura/estado
TITULOS = {
    "s1": "Estado inicial (apos insercoes)",
    "s2": "Estado intermediario (bifurcacao / rotacao / split)",
    "s3": "Estado apos remocao / reorganizacao",
}
NOMES = {
    "trie": "Trie", "patricia": "Patricia (Radix compacta)",
    "splay": "Arvore Splay", "treap": "Arvore Treap", "kdtree": "KD-Tree",
}

def parse(path):
    labels, children, parent = {}, {}, {}
    edgelab = {}
    with open(path) as f:
        for line in f:
            parts = line.rstrip("\n").split(" ", 3)
            if not parts or parts[0] == "":
                continue
            if parts[0] == "N":
                nid = int(parts[1])
                labels[nid] = parts[2] if len(parts) > 2 else "-"
                children.setdefault(nid, [])
            elif parts[0] == "E":
                pid, cid = int(parts[1]), int(parts[2])
                lab = parts[3] if len(parts) > 3 else "-"
                children.setdefault(pid, []).append(cid)
                parent[cid] = pid
                edgelab[(pid, cid)] = lab
    root = next((n for n in labels if n not in parent), 0)
    return labels, children, edgelab, root

def layout(children, edgelab, root):
    pos = {}
    nx = [0]
    def assign(nid, depth):
        ch = children.get(nid, [])
        if not ch:
            x = nx[0]; nx[0] += 1
            pos[nid] = (x, depth); return x
        labs = [edgelab.get((nid, c), "-") for c in ch]
        binary = set(labs) <= {"L", "R"}
        if binary:
            lc = next((c for c in ch if edgelab.get((nid, c)) == "L"), None)
            rc = next((c for c in ch if edgelab.get((nid, c)) == "R"), None)
            xs = []
            if lc is not None: xs.append(assign(lc, depth + 1))
            else: xs.append(nx[0]); nx[0] += 1          # fantasma a esquerda
            if rc is not None: xs.append(assign(rc, depth + 1))
            else: xs.append(nx[0]); nx[0] += 1          # fantasma a direita
        else:
            xs = [assign(c, depth + 1) for c in ch]
        x = (min(xs) + max(xs)) / 2.0
        pos[nid] = (x, depth); return x
    assign(root, 0)
    return pos

def cor(label):
    if label == "*":  return "#8fd19e", "#2f7d46"   # fim de palavra (verde)
    if label == "-":  return "#e6e6e6", "#9aa0a6"   # no interno vazio (cinza)
    return "#a9cce3", "#1f618d"                       # no com valor (azul)

def desenhar(path):
    labels, children, edgelab, root = parse(path)
    pos = layout(children, edgelab, root)
    if not pos:
        return
    maxd = max(d for _, d in pos.values())
    maxx = max(x for x, _ in pos.values())
    largura = max(6, (maxx + 1) * 1.15)
    altura = max(3, (maxd + 1) * 1.25)
    fig, ax = plt.subplots(figsize=(largura, altura))

    # arestas
    for (pid, cid), lab in edgelab.items():
        x1, d1 = pos[pid]; x2, d2 = pos[cid]
        ax.plot([x1, x2], [-d1, -d2], color="#95a5a6", lw=1.4, zorder=1)
        if lab not in ("L", "R", "-"):                # rotulo textual (substring/char)
            ax.text((x1 + x2) / 2, (-d1 - d2) / 2, lab, fontsize=10,
                    ha="center", va="center", color="#c0392b", fontweight="bold",
                    bbox=dict(boxstyle="round,pad=0.12", fc="white", ec="none"))

    # nos
    for nid, (x, d) in pos.items():
        face, edge = cor(labels[nid])
        txt = labels[nid]
        r = 0.34
        ax.add_patch(plt.Circle((x, -d), r, facecolor=face, edgecolor=edge,
                                 lw=1.6, zorder=2))
        mostra = "" if txt == "-" else txt
        ax.text(x, -d, mostra, fontsize=9, ha="center", va="center",
                zorder=3, fontweight="bold")

    base = os.path.basename(path).replace(".tree", "")
    est, s = base.split("_")
    titulo = f"{NOMES.get(est, est)} — {TITULOS.get(s, s)}"
    ax.set_title(titulo, fontsize=12, fontweight="bold", pad=12)
    ax.set_xlim(-0.8, maxx + 0.8)
    ax.set_ylim(-maxd - 0.8, 0.9)
    ax.axis("off")
    fig.tight_layout()
    out = os.path.join(OUT, base + ".png")
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print("gerado", out)

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for p in sorted(glob.glob(os.path.join(DATA, "*.tree"))):
        desenhar(p)
