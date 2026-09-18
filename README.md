# Trabalho Prático I — Estruturas em Árvores Avançadas

Implementação, visualização e análise experimental de **5 estruturas de dados
hierárquicas** em C++17: **Trie**, **Árvore Patricia (Radix compacta)**,
**Árvore Splay**, **Árvore Treap** e **KD-Tree**.

> Disciplina: Estrutura de Dados — 5º semestre.
> Relatório técnico em `relatorio/` (versão editável `.docx` e `.pdf`).

## Estrutura do repositório

```
include/         implementações (header-only, comentadas em português)
  trie.hpp       Trie (árvore de prefixos)
  patricia.hpp   Árvore Patricia (radix tree compacta)
  splay.hpp      Árvore Splay (autoajustável)
  treap.hpp      Árvore Treap (BST + max-heap por prioridade)
  kdtree.hpp     KD-Tree 2D (busca espacial / vizinho mais próximo)
src/
  gen_figs.cpp   demonstração das operações + gera os dados das figuras
  bench.cpp      experimentos de desempenho (gera os CSVs)
tools/
  treeviz.py     desenha as árvores (PNG) a partir dos .tree
  kdviz.py       desenha o particionamento do plano da KD-Tree
  plot_bench.py  gera os gráficos dos experimentos a partir dos CSVs
figs/            imagens geradas (estados das árvores e gráficos)
bench_out/       resultados dos experimentos (CSV)
relatorio/       relatório técnico (.docx e .pdf)
```

## Como compilar e reproduzir tudo

Requisitos: `g++` (C++17), `python3` com `matplotlib` e `numpy`.

```bash
make            # compila gen_figs e bench
make figs       # roda a demonstração e gera os dados das figuras
make bench      # roda os experimentos e gera os CSVs
make imagens    # gera TODAS as imagens PNG (figuras + gráficos)
```

Ou simplesmente:

```bash
make imagens    # faz tudo: compila, roda e gera as imagens
```

## Operações implementadas por estrutura

| Estrutura | Inserção | Busca | Remoção | Operações específicas |
|-----------|:--------:|:-----:|:-------:|-----------------------|
| Trie      | ✔ | ✔ | ✔ | busca por prefixo (`comecaCom`) |
| Patricia  | ✔ | ✔ | ✔ | split / merge de arestas |
| Splay     | ✔ | ✔ | ✔ | splay (zig / zig-zig / zig-zag) |
| Treap     | ✔ | ✔ | ✔ | rotações por prioridade |
| KD-Tree   | ✔ | ✔ | ✔ | vizinho mais próximo (poda) |

Reprodutibilidade: as entradas pseudoaleatórias usam semente fixa (`42`).
