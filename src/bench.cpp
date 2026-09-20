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

using Clock = std::chrono::high_resolution_clock;
static double ms(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

volatile long long g_sink = 0;

std::string randomString(std::mt19937& rng, int len) {
    static const char abc[] = "abcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<int> d(0, 25);
    std::string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i) s += abc[d(rng)];
    return s;
}

int main() {
    std::mt19937 rng(42);

    {
        std::ofstream f("bench_out/bench_str.csv");
        f << "n,trie_ins,trie_bus,pat_ins,pat_bus\n";
        for (int n = 2000; n <= 20000; n += 2000) {
            std::vector<std::string> keys;
            keys.reserve(n);
            for (int i = 0; i < n; ++i)
                keys.push_back(randomString(rng, 8));

            Trie trie; Patricia pat;
            long long acc = 0;
            auto t0 = Clock::now();
            for (auto& s : keys) trie.insert(s);
            auto t1 = Clock::now();
            for (auto& s : keys) acc += trie.search(s);
            auto t2 = Clock::now();
            for (auto& s : keys) pat.insert(s);
            auto t3 = Clock::now();
            for (auto& s : keys) acc += pat.search(s);
            auto t4 = Clock::now();
            g_sink += acc;

            f << n << "," << ms(t0,t1) << "," << ms(t1,t2) << ","
              << ms(t2,t3) << "," << ms(t3,t4) << "\n";
        }
        std::cout << "bench_str.csv gerado\n";
    }

    {
        std::ofstream f("bench_out/bench_int.csv");
        f << "n,splay_ins,splay_bus,treap_ins,treap_bus,set_ins,set_bus\n";
        for (int n = 2000; n <= 20000; n += 2000) {
            std::vector<int> keys;
            keys.reserve(n);
            std::uniform_int_distribution<int> d(0, 10'000'000);
            for (int i = 0; i < n; ++i) keys.push_back(d(rng));

            Splay sp; Treap tr(42); std::set<int> st;
            long long acc = 0;

            auto t0 = Clock::now();
            for (int x : keys) sp.insert(x);
            auto t1 = Clock::now();
            for (int x : keys) acc += sp.search(x);
            auto t2 = Clock::now();

            for (int x : keys) tr.insert(x);
            auto t3 = Clock::now();
            for (int x : keys) acc += tr.search(x);
            auto t4 = Clock::now();

            for (int x : keys) st.insert(x);
            auto t5 = Clock::now();
            for (int x : keys) acc += st.count(x);
            auto t6 = Clock::now();
            g_sink += acc;

            f << n << "," << ms(t0,t1) << "," << ms(t1,t2) << ","
              << ms(t2,t3) << "," << ms(t3,t4) << ","
              << ms(t4,t5) << "," << ms(t5,t6) << "\n";
        }
        std::cout << "bench_int.csv gerado\n";
    }

    {
        std::ofstream f("bench_out/bench_locality.csv");
        f << "p,splay_ms,treap_ms,set_ms\n";
        const int M = 50000;
        const int H = 100;
        const int Q = 500000;
        std::vector<double> ps = {0.0, 0.3, 0.5, 0.7, 0.8, 0.9, 0.95, 0.99};

        for (double p : ps) {
            Splay sp; Treap tr(42); std::set<int> st;
            for (int i = 0; i < M; ++i) { sp.insert(i); tr.insert(i); st.insert(i); }

            std::mt19937 qr(7);
            std::uniform_real_distribution<double> coin(0.0, 1.0);
            std::uniform_int_distribution<int> hot(0, H - 1);
            std::uniform_int_distribution<int> any(0, M - 1);
            std::vector<int> queries(Q);
            for (int i = 0; i < Q; ++i)
                queries[i] = (coin(qr) < p) ? hot(qr) : any(qr);

            long long acc = 0;
            auto t0 = Clock::now();
            for (int x : queries) acc += sp.search(x);
            auto t1 = Clock::now();
            for (int x : queries) acc += tr.search(x);
            auto t2 = Clock::now();
            for (int x : queries) acc += st.count(x);
            auto t3 = Clock::now();
            g_sink += acc;

            f << p << "," << ms(t0,t1) << "," << ms(t1,t2) << "," << ms(t2,t3) << "\n";
        }
        std::cout << "bench_locality.csv gerado\n";
    }

    {
        std::ofstream f("bench_out/bench_kd.csv");
        f << "n,kd_ms,linear_ms\n";
        std::uniform_real_distribution<double> d(0.0, 10000.0);
        const int Q = 2000;
        for (int n = 1000; n <= 20000; n += 2000) {
            std::vector<KDTree::Point> pts(n);
            for (int i = 0; i < n; ++i) pts[i] = {d(rng), d(rng)};

            KDTree kd;
            for (auto& p : pts) kd.insert(p);

            std::vector<KDTree::Point> targets(Q);
            for (int i = 0; i < Q; ++i) targets[i] = {d(rng), d(rng)};

            auto t0 = Clock::now();
            for (auto& a : targets) { volatile auto r = kd.nearestNeighbor(a); (void)r; }
            auto t1 = Clock::now();
            for (auto& a : targets) {
                double best = 1e30;
                for (auto& p : pts) {
                    double dx = p.x - a.x, dy = p.y - a.y;
                    double d2 = dx*dx + dy*dy;
                    if (d2 < best) best = d2;
                }
                volatile double r = best; (void)r;
            }
            auto t2 = Clock::now();

            f << n << "," << ms(t0,t1) << "," << ms(t1,t2) << "\n";
        }
        std::cout << "bench_kd.csv gerado\n";
    }

    std::cout << "Experimentos concluidos. CSVs em bench_out/\n";
    return 0;
}
