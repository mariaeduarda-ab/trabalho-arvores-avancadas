#include "../include/trie.hpp"
#include "../include/patricia.hpp"
#include "../include/splay.hpp"
#include "../include/treap.hpp"
#include "../include/kdtree.hpp"

#include <fstream>
#include <iostream>
#include <string>

static const std::string DIR = "figs/data/";

template <class Tree>
void save(const Tree& tree, const std::string& name) {
    std::ofstream f(DIR + name);
    tree.writeTree(f);
}

void saveSpace(const KDTree& kd, const std::string& name) {
    std::ofstream f(DIR + name);
    kd.writeSpace(f, 0, 100, 0, 100);
}

std::string sn(bool b) { return b ? "sim" : "nao"; }

int main() {
    {
        std::cout << "----- TRIE -----\n";
        Trie t;
        t.insert("casa"); t.insert("caso");
        save(t, "trie_s1.tree");
        std::cout << "S1 inseridas: casa, caso\n";
        t.insert("carro");
        save(t, "trie_s2.tree");
        std::cout << "S2 inserida: carro (nova bifurcacao)\n";
        std::cout << "  busca 'casa' = " << sn(t.search("casa"))
                  << " | prefixo 'car' = " << sn(t.startsWith("car"))
                  << " | palavra 'cas' = " << sn(t.search("cas")) << "\n";
        t.remove("caso");
        save(t, "trie_s3.tree");
        std::cout << "S3 removida: caso | busca 'caso' = " << sn(t.search("caso")) << "\n\n";
    }

    {
        std::cout << "----- PATRICIA -----\n";
        Patricia p;
        p.insert("corda"); p.insert("cordao"); p.insert("corte");
        save(p, "patricia_s1.tree");
        std::cout << "S1 inseridas: corda, cordao, corte\n";
        p.insert("casa");
        save(p, "patricia_s2.tree");
        std::cout << "S2 inserida: casa (split de prefixo em 'c')\n";
        std::cout << "  busca 'cordao' = " << sn(p.search("cordao"))
                  << " | busca 'cord' = " << sn(p.search("cord")) << "\n";
        p.remove("corte");
        save(p, "patricia_s3.tree");
        std::cout << "S3 removida: corte | busca 'corte' = " << sn(p.search("corte")) << "\n\n";
    }

    {
        std::cout << "----- SPLAY -----\n";
        Splay s;
        int keys[] = {50, 30, 70, 20, 40, 60, 80};
        for (int x : keys) s.insert(x);
        save(s, "splay_s1.tree");
        std::cout << "S1 inseridos: 50 30 70 20 40 60 80 (raiz = ultimo acessado)\n";
        s.search(20);
        save(s, "splay_s2.tree");
        std::cout << "S2 busca 20: no acessado vai para a raiz por rotacoes\n";
        s.remove(20);
        save(s, "splay_s3.tree");
        std::cout << "S3 removido 20 | busca 20 = " << sn(s.search(20))
                  << " | busca 60 = " << sn(s.search(60)) << "\n\n";
    }

    {
        std::cout << "----- TREAP -----\n";
        Treap tr(42);
        int keys[] = {50, 30, 70, 20, 40};
        for (int x : keys) tr.insert(x);
        save(tr, "treap_s1.tree");
        std::cout << "S1 inseridos: 50 30 70 20 40 (prioridades aleatorias, seed=42)\n";
        tr.insert(35); tr.insert(80);
        save(tr, "treap_s2.tree");
        std::cout << "S2 inseridos: 35, 80 (rotacoes para manter o max-heap)\n";
        std::cout << "  busca 35 = " << sn(tr.search(35))
                  << " | busca 99 = " << sn(tr.search(99)) << "\n";
        tr.remove(30);
        save(tr, "treap_s3.tree");
        std::cout << "S3 removido 30 | busca 30 = " << sn(tr.search(30)) << "\n\n";
    }

    {
        std::cout << "----- KD-TREE (2D) -----\n";
        KDTree kd;
        KDTree::Point pts[] = {{30,40},{10,70},{70,20},{50,90},{80,60},{20,10}};
        for (auto& p : pts) kd.insert(p);
        save(kd, "kdtree_s1.tree");
        saveSpace(kd, "kdtree_s1.space");
        std::cout << "S1 inseridos 6 pontos (raiz=(30,40) corta em x)\n";
        kd.insert(60, 50);
        save(kd, "kdtree_s2.tree");
        saveSpace(kd, "kdtree_s2.space");
        std::cout << "S2 inserido (60,50): nova subdivisao do plano\n";
        KDTree::Point target{55, 55};
        KDTree::Point nn = kd.nearestNeighbor(target);
        std::cout << "  vizinho mais proximo de (55,55) = ("
                  << (int)nn.x << "," << (int)nn.y << ")\n";
        kd.remove({30, 40});
        save(kd, "kdtree_s3.tree");
        saveSpace(kd, "kdtree_s3.space");
        std::cout << "S3 removido (30,40) | busca (30,40) = " << sn(kd.search({30,40}))
                  << " | busca (80,60) = " << sn(kd.search({80,60})) << "\n\n";
    }

    std::cout << "Arquivos de dados gerados em " << DIR << "\n";
    return 0;
}
