// ===========================================================================
//  treap.hpp  -  Arvore Treap (Tree + Heap)
//  Trabalho Pratico I - Estruturas em Arvores Avancadas
//
//  A Treap combina duas propriedades ao mesmo tempo:
//    (1) BST pelas CHAVES     -> esq < no < dir  (busca binaria comum)
//    (2) MAX-HEAP pelas PRIORIDADES -> prioridade do pai >= prioridade dos filhos
//
//  A cada no e sorteada uma PRIORIDADE aleatoria. Como as prioridades sao
//  aleatorias, a arvore fica balanceada em PROBABILIDADE (altura esperada
//  O(log n)) sem precisar de regras complexas como AVL/rubro-negra.
//
//  O equilibrio entre as duas propriedades e mantido por ROTACOES simples
//  durante a insercao e a remocao.
// ===========================================================================
#ifndef TREAP_HPP
#define TREAP_HPP

#include <ostream>
#include <random>

class Treap {
private:
    struct No {
        int chave;
        int prioridade;
        No *esq = nullptr, *dir = nullptr;
        No(int c, int p) : chave(c), prioridade(p) {}
    };

    No* raiz = nullptr;
    std::mt19937 rng;                     // gerador com semente fixa -> reproduzivel
    std::uniform_int_distribution<int> dist{1, 999};

    void destruir(No* n) {
        if (!n) return;
        destruir(n->esq);
        destruir(n->dir);
        delete n;
    }

    // rotacao a direita: o filho esquerdo 'x' sobe e vira raiz da subarvore
    No* rotDir(No* y) {
        No* x = y->esq;
        y->esq = x->dir;
        x->dir = y;
        return x;
    }

    // rotacao a esquerda: o filho direito 'y' sobe e vira raiz da subarvore
    No* rotEsq(No* x) {
        No* y = x->dir;
        x->dir = y->esq;
        y->esq = x;
        return y;
    }

    No* inserirRec(No* n, int chave) {
        if (!n) return new No(chave, dist(rng));
        if (chave < n->chave) {
            n->esq = inserirRec(n->esq, chave);
            // se o filho tem prioridade maior, ele deve subir (max-heap)
            if (n->esq->prioridade > n->prioridade) n = rotDir(n);
        } else if (chave > n->chave) {
            n->dir = inserirRec(n->dir, chave);
            if (n->dir->prioridade > n->prioridade) n = rotEsq(n);
        }
        // chave igual: ignora (sem duplicatas)
        return n;
    }

    No* removerRec(No* n, int chave, bool& achou) {
        if (!n) return nullptr;
        if (chave < n->chave) {
            n->esq = removerRec(n->esq, chave, achou);
        } else if (chave > n->chave) {
            n->dir = removerRec(n->dir, chave, achou);
        } else {
            achou = true;
            if (!n->esq) { No* t = n->dir; delete n; return t; }
            if (!n->dir) { No* t = n->esq; delete n; return t; }
            // dois filhos: rotaciona o filho de MAIOR prioridade para cima
            // e continua descendo o no ate virar folha
            if (n->esq->prioridade > n->dir->prioridade) {
                n = rotDir(n);
                n->dir = removerRec(n->dir, chave, achou);
            } else {
                n = rotEsq(n);
                n->esq = removerRec(n->esq, chave, achou);
            }
        }
        return n;
    }

    bool buscarRec(No* n, int chave) const {
        while (n) {
            if (chave == n->chave) return true;
            n = (chave < n->chave) ? n->esq : n->dir;
        }
        return false;
    }

    int escreverRec(No* n, std::ostream& os, int& proxId,
                    const char* lado, int idPai) const {
        int meuId = proxId++;
        // rotulo mostra a chave e a prioridade: chave(pPrioridade)
        os << "N " << meuId << " " << n->chave << "(p" << n->prioridade << ")\n";
        if (idPai >= 0) os << "E " << idPai << " " << meuId << " " << lado << "\n";
        if (n->esq) escreverRec(n->esq, os, proxId, "L", meuId);
        if (n->dir) escreverRec(n->dir, os, proxId, "R", meuId);
        return meuId;
    }

public:
    explicit Treap(unsigned semente = 42) : rng(semente) {}
    ~Treap() { destruir(raiz); }

    void inserir(int chave) { raiz = inserirRec(raiz, chave); }
    bool buscar(int chave) const { return buscarRec(raiz, chave); }
    bool remover(int chave) {
        bool achou = false;
        raiz = removerRec(raiz, chave, achou);
        return achou;
    }

    void escreverArvore(std::ostream& os) const {
        int proxId = 0;
        if (raiz) escreverRec(raiz, os, proxId, "-", -1);
    }
};

#endif // TREAP_HPP
