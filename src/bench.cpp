// ===========================================================================
//  bench.cpp  -  Experimentos computacionais (Secao 5 do relatorio)
//
//  Gera 4 arquivos CSV em bench_out/, cada um base de um grafico:
//    1) bench_str.csv      Trie x Patricia  (insercao/busca de strings)
//    2) bench_int.csv      Splay x Treap x std::set (BST balanceada) - inteiros
//    3) bench_locality.csv Splay x Treap x std::set sob acesso com LOCALIDADE
//    4) bench_kd.csv        KD-Tree x busca linear (vizinho mais proximo)
//
//  Metodologia: tempos medidos com std::chrono, entradas pseudoaleatorias com
//  semente fixa (reproduzivel). Cada tempo e a soma de N operacoes, em ms.
// ===========================================================================
#include "../include/trie.hpp"
#include "../include/patricia.hpp"
#include "../include/splay.hpp"
#include "../include/treap.hpp"
#include "../include/kdtree.hpp"

#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <iostream>

using Relogio = std::chrono::high_resolution_clock;
static double ms(Relogio::time_point a, Relogio::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

// "sumidouro" volatil: impede o compilador (-O2) de eliminar os lacos de busca
// cujo resultado nao seria usado. Acumulamos os resultados aqui.
volatile long long g_sink = 0;

std::string strAleatoria(std::mt19937& rng, int tam) {
    static const char abc[] = "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<int> d(0, 25);
    std::string s;
    s.reserve(tam);
    for (int i = 0; i < tam; ++i) s += abc[d(rng)];
    return s;
}

int main() {
    std::mt19937 rng(42);

    // ---------- 1) STRINGS: Trie x Patricia ------------------------------
    {
        std::ofstream f("bench_out/bench_str.csv");
        f << "n,trie_ins,trie_bus,pat_ins,pat_bus\n";
        for (int n = 2000; n <= 20000; n += 2000) {
            std::vector<std::string> chaves;
            chaves.reserve(n);
            for (int i = 0; i < n; ++i)
                chaves.push_back(strAleatoria(rng, 8));

            Trie trie; Patricia pat;
            long long acc = 0;
            auto t0 = Relogio::now();
            for (auto& s : chaves) trie.inserir(s);
            auto t1 = Relogio::now();
            for (auto& s : chaves) acc += trie.buscar(s);
            auto t2 = Relogio::now();
            for (auto& s : chaves) pat.inserir(s);
            auto t3 = Relogio::now();
            for (auto& s : chaves) acc += pat.buscar(s);
            auto t4 = Relogio::now();
            g_sink += acc;

            f << n << "," << ms(t0,t1) << "," << ms(t1,t2) << ","
              << ms(t2,t3) << "," << ms(t3,t4) << "\n";
        }
        std::cout << "bench_str.csv ok\n";
    }

    // ---------- 2) INTEIROS: Splay x Treap x std::set --------------------
    {
        std::ofstream f("bench_out/bench_int.csv");
        f << "n,splay_ins,splay_bus,treap_ins,treap_bus,set_ins,set_bus\n";
        for (int n = 2000; n <= 20000; n += 2000) {
            std::vector<int> chaves;
            chaves.reserve(n);
            std::uniform_int_distribution<int> d(0, 10'000'000);
            for (int i = 0; i < n; ++i) chaves.push_back(d(rng));

            Splay sp; Treap tr(42); std::set<int> st;
            long long acc = 0;

            auto t0 = Relogio::now();
            for (int x : chaves) sp.inserir(x);
            auto t1 = Relogio::now();
            for (int x : chaves) acc += sp.buscar(x);
            auto t2 = Relogio::now();

            for (int x : chaves) tr.inserir(x);
            auto t3 = Relogio::now();
            for (int x : chaves) acc += tr.buscar(x);
            auto t4 = Relogio::now();

            for (int x : chaves) st.insert(x);
            auto t5 = Relogio::now();
            for (int x : chaves) acc += st.count(x);
            auto t6 = Relogio::now();
            g_sink += acc;

            f << n << "," << ms(t0,t1) << "," << ms(t1,t2) << ","
              << ms(t2,t3) << "," << ms(t3,t4) << ","
              << ms(t4,t5) << "," << ms(t5,t6) << "\n";
        }
        std::cout << "bench_int.csv ok\n";
    }

    // ---------- 3) LOCALIDADE DE ACESSO: vantagem da Splay ---------------
    {
        std::ofstream f("bench_out/bench_locality.csv");
        f << "p,splay_ms,treap_ms,set_ms\n";
        const int M = 50000;    // tamanho da estrutura
        const int H = 100;      // "hot set": chaves acessadas com frequencia
        const int Q = 500000;   // numero de consultas
        std::vector<double> ps = {0.0, 0.3, 0.5, 0.7, 0.8, 0.9, 0.95, 0.99};

        for (double p : ps) {
            Splay sp; Treap tr(42); std::set<int> st;
            for (int i = 0; i < M; ++i) { sp.inserir(i); tr.inserir(i); st.insert(i); }

            // pre-gera as consultas: com prob p vem do hot set, senao uniforme
            std::mt19937 qr(7);
            std::uniform_real_distribution<double> moeda(0.0, 1.0);
            std::uniform_int_distribution<int> hot(0, H - 1);
            std::uniform_int_distribution<int> qualquer(0, M - 1);
            std::vector<int> consultas(Q);
            for (int i = 0; i < Q; ++i)
                consultas[i] = (moeda(qr) < p) ? hot(qr) : qualquer(qr);

            long long acc = 0;
            auto t0 = Relogio::now();
            for (int x : consultas) acc += sp.buscar(x);
            auto t1 = Relogio::now();
            for (int x : consultas) acc += tr.buscar(x);
            auto t2 = Relogio::now();
            for (int x : consultas) acc += st.count(x);
            auto t3 = Relogio::now();
            g_sink += acc;

            f << p << "," << ms(t0,t1) << "," << ms(t1,t2) << "," << ms(t2,t3) << "\n";
        }
        std::cout << "bench_locality.csv ok\n";
    }

    // ---------- 4) KD-TREE x busca linear (vizinho mais proximo) ---------
    {
        std::ofstream f("bench_out/bench_kd.csv");
        f << "n,kd_ms,linear_ms\n";
        std::uniform_real_distribution<double> d(0.0, 10000.0);
        const int Q = 2000;   // consultas de vizinho mais proximo
        for (int n = 1000; n <= 20000; n += 2000) {
            std::vector<KDTree::Ponto> pts(n);
            for (int i = 0; i < n; ++i) pts[i] = {d(rng), d(rng)};

            KDTree kd;
            for (auto& p : pts) kd.inserir(p);

            std::vector<KDTree::Ponto> alvos(Q);
            for (int i = 0; i < Q; ++i) alvos[i] = {d(rng), d(rng)};

            auto t0 = Relogio::now();
            for (auto& a : alvos) { volatile auto r = kd.vizinhoMaisProximo(a); (void)r; }
            auto t1 = Relogio::now();
            // busca linear (forca bruta) para comparar
            for (auto& a : alvos) {
                double melhor = 1e30;
                for (auto& p : pts) {
                    double dx = p.x - a.x, dy = p.y - a.y;
                    double d2 = dx*dx + dy*dy;
                    if (d2 < melhor) melhor = d2;
                }
                volatile double r = melhor; (void)r;
            }
            auto t2 = Relogio::now();

            f << n << "," << ms(t0,t1) << "," << ms(t1,t2) << "\n";
        }
        std::cout << "bench_kd.csv ok\n";
    }

    std::cout << "Experimentos concluidos. CSVs em bench_out/\n";
    return 0;
}
