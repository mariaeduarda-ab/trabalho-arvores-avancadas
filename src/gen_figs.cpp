// ===========================================================================
//  gen_figs.cpp  -  Gera os arquivos de dados das figuras (estados das arvores)
//  e imprime um log de demonstracao/verificacao das operacoes.
//
//  Para cada estrutura sao gerados 3 estados:
//    s1 = estado inicial (apos um conjunto de insercoes)
//    s2 = estado intermediario (evidenciando bifurcacao / split / rotacao)
//    s3 = estado apos uma remocao / reorganizacao
//
//  Os arquivos .tree/.space sao lidos pelos scripts em tools/ que geram os PNG.
// ===========================================================================
#include "../include/trie.hpp"
#include "../include/patricia.hpp"
#include "../include/splay.hpp"
#include "../include/treap.hpp"
#include "../include/kdtree.hpp"

#include <fstream>
#include <iostream>
#include <string>

static const std::string DIR = "figs/data/";

template <class Arvore>
void salvar(const Arvore& arv, const std::string& nome) {
    std::ofstream f(DIR + nome);
    arv.escreverArvore(f);
}

void salvarEspaco(const KDTree& kd, const std::string& nome) {
    std::ofstream f(DIR + nome);
    kd.escreverEspaco(f, 0, 100, 0, 100);
}

std::string sn(bool b) { return b ? "sim" : "nao"; }

int main() {
    std::cout << "==================================================\n";
    std::cout << " DEMONSTRACAO E VERIFICACAO DAS 5 ESTRUTURAS\n";
    std::cout << "==================================================\n\n";

    // ------------------------------------------------------------------ TRIE
    {
        std::cout << "----- TRIE -----\n";
        Trie t;
        t.inserir("casa"); t.inserir("caso");
        salvar(t, "trie_s1.tree");
        std::cout << "S1 inseridas: casa, caso\n";
        t.inserir("carro");                          // bifurcacao em 'ca'
        salvar(t, "trie_s2.tree");
        std::cout << "S2 inserida:  carro (nova bifurcacao)\n";
        std::cout << "  busca 'casa'  = " << sn(t.buscar("casa"))
                  << " | busca 'car' (prefixo) = " << sn(t.comecaCom("car"))
                  << " | busca 'cas' (palavra) = " << sn(t.buscar("cas")) << "\n";
        t.remover("caso");
        salvar(t, "trie_s3.tree");
        std::cout << "S3 removida:  caso | busca 'caso' = " << sn(t.buscar("caso")) << "\n\n";
    }

    // -------------------------------------------------------------- PATRICIA
    {
        std::cout << "----- PATRICIA (Radix compacta) -----\n";
        Patricia p;
        p.inserir("corda"); p.inserir("cordao"); p.inserir("corte");
        salvar(p, "patricia_s1.tree");
        std::cout << "S1 inseridas: corda, cordao, corte\n";
        p.inserir("casa");                           // split em 'c'
        salvar(p, "patricia_s2.tree");
        std::cout << "S2 inserida:  casa (split de prefixo em 'c')\n";
        std::cout << "  busca 'cordao' = " << sn(p.buscar("cordao"))
                  << " | busca 'cord' = " << sn(p.buscar("cord")) << "\n";
        p.remover("corte");                          // pode fundir nos (merge)
        salvar(p, "patricia_s3.tree");
        std::cout << "S3 removida:  corte | busca 'corte' = " << sn(p.buscar("corte")) << "\n\n";
    }

    // ----------------------------------------------------------------- SPLAY
    {
        std::cout << "----- SPLAY -----\n";
        Splay s;
        int ins[] = {50, 30, 70, 20, 40, 60, 80};
        for (int x : ins) s.inserir(x);
        salvar(s, "splay_s1.tree");
        std::cout << "S1 inseridos: 50 30 70 20 40 60 80 (raiz = ultimo acessado)\n";
        s.buscar(20);                                // 20 sobe para a raiz (splay)
        salvar(s, "splay_s2.tree");
        std::cout << "S2 busca 20: no acessado vai para a raiz por rotacoes\n";
        s.remover(20);
        salvar(s, "splay_s3.tree");
        std::cout << "S3 removido 20 | busca 20 = " << sn(s.buscar(20))
                  << " | busca 60 = " << sn(s.buscar(60)) << "\n\n";
    }

    // ----------------------------------------------------------------- TREAP
    {
        std::cout << "----- TREAP -----\n";
        Treap tr(42);
        int ins[] = {50, 30, 70, 20, 40};
        for (int x : ins) tr.inserir(x);
        salvar(tr, "treap_s1.tree");
        std::cout << "S1 inseridos: 50 30 70 20 40 (prioridades aleatorias, seed=42)\n";
        tr.inserir(35); tr.inserir(80);              // rotacoes por prioridade
        salvar(tr, "treap_s2.tree");
        std::cout << "S2 inseridos: 35, 80 (rotacoes para manter o max-heap)\n";
        std::cout << "  busca 35 = " << sn(tr.buscar(35))
                  << " | busca 99 = " << sn(tr.buscar(99)) << "\n";
        tr.remover(30);
        salvar(tr, "treap_s3.tree");
        std::cout << "S3 removido 30 | busca 30 = " << sn(tr.buscar(30)) << "\n\n";
    }

    // --------------------------------------------------------------- KD-TREE
    {
        std::cout << "----- KD-TREE (2D) -----\n";
        KDTree kd;
        KDTree::Ponto pts[] = {{30,40},{10,70},{70,20},{50,90},{80,60},{20,10}};
        for (auto& p : pts) kd.inserir(p);
        salvar(kd, "kdtree_s1.tree");
        salvarEspaco(kd, "kdtree_s1.space");
        std::cout << "S1 inseridos 6 pontos (raiz=(30,40) corta em x)\n";
        kd.inserir(60, 50);                          // novo particionamento
        salvar(kd, "kdtree_s2.tree");
        salvarEspaco(kd, "kdtree_s2.space");
        std::cout << "S2 inserido (60,50): nova subdivisao do plano\n";
        KDTree::Ponto alvo{55, 55};
        KDTree::Ponto vmp = kd.vizinhoMaisProximo(alvo);
        std::cout << "  vizinho mais proximo de (55,55) = ("
                  << (int)vmp.x << "," << (int)vmp.y << ")\n";
        kd.remover({30, 40});                        // remove a raiz (usa findMin)
        salvar(kd, "kdtree_s3.tree");
        salvarEspaco(kd, "kdtree_s3.space");
        std::cout << "S3 removido (30,40) | busca (30,40) = " << sn(kd.buscar({30,40}))
                  << " | busca (80,60) = " << sn(kd.buscar({80,60})) << "\n\n";
    }

    std::cout << "Arquivos de dados gerados em " << DIR << "\n";
    return 0;
}
