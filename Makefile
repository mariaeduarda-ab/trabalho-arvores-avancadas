# Makefile - Trabalho Pratico I: Estruturas em Arvores Avancadas
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
BIN      = bin

all: $(BIN)/gen_figs $(BIN)/bench

$(BIN):
	mkdir -p $(BIN)

$(BIN)/gen_figs: src/gen_figs.cpp include/*.hpp | $(BIN)
	$(CXX) $(CXXFLAGS) src/gen_figs.cpp -o $@

$(BIN)/bench: src/bench.cpp include/*.hpp | $(BIN)
	$(CXX) $(CXXFLAGS) src/bench.cpp -o $@

# gera os dados das figuras (estados das arvores)
figs: $(BIN)/gen_figs
	mkdir -p figs/data
	./$(BIN)/gen_figs

# roda os experimentos e gera os CSVs
bench: $(BIN)/bench
	mkdir -p bench_out
	./$(BIN)/bench

# gera todas as imagens PNG (precisa de python3 + matplotlib + numpy)
imagens: figs bench
	python3 tools/treeviz.py
	python3 tools/kdviz.py
	python3 tools/plot_bench.py

clean:
	rm -rf $(BIN) figs/data/*.tree figs/data/*.space bench_out/*.csv

.PHONY: all figs bench imagens clean
