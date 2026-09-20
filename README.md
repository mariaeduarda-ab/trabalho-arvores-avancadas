# Trabalho Prático I — Estruturas em Árvores Avançadas

Implementação, visualização e análise experimental de **5 estruturas de dados
hierárquicas** em C++17: **Trie**, **Árvore Patricia (Radix compacta)**,
**Árvore Splay**, **Árvore Treap** e **KD-Tree**.

> Disciplina: Algoritmos e Estruturas de Dados I (AEDS I).
> Relatório técnico em `relatorio/` (`.pdf` e versão editável `.docx`).

## Estrutura do repositório

```
include/         implementações (header-only) das 5 estruturas
  trie.hpp       Trie (árvore de prefixos)
  patricia.hpp   Árvore Patricia (radix tree compacta)
  splay.hpp      Árvore Splay (autoajustável)
  treap.hpp      Árvore Treap (BST + max-heap por prioridade)
  kdtree.hpp     KD-Tree 2D (busca espacial / vizinho mais próximo)
src/
  gen_figs.cpp   demonstração das operações + gera os dados das figuras
  bench.cpp      experimentos de desempenho (gera os CSVs)
figs/            imagens do relatório (estados das árvores e gráficos)
bench_out/       resultados dos experimentos (CSV)
relatorio/       relatório técnico (.pdf e .docx)
```

## Como compilar e executar

Requisitos: `g++` (C++17).

```bash
make            # compila gen_figs e bench
make figs       # roda a demonstração e gera os dados das figuras
make bench      # roda os experimentos e gera os CSVs
```

## Operações implementadas por estrutura

| Estrutura | Inserção | Busca | Remoção | Operações específicas |
|-----------|:--------:|:-----:|:-------:|-----------------------|
| Trie      | ✔ | ✔ | ✔ | busca por prefixo (`startsWith`) |
| Patricia  | ✔ | ✔ | ✔ | split / merge de arestas |
| Splay     | ✔ | ✔ | ✔ | splay (zig / zig-zig / zig-zag) |
| Treap     | ✔ | ✔ | ✔ | rotações por prioridade |
| KD-Tree   | ✔ | ✔ | ✔ | vizinho mais próximo (poda) |

Reprodutibilidade: as entradas pseudoaleatórias usam semente fixa (`42`).
