// ===========================================================================
//  splay.hpp  -  Arvore Splay
//  Trabalho Pratico I - Estruturas em Arvores Avancadas
//
//  A Splay e uma arvore binaria de busca AUTOAJUSTAVEL: a cada acesso
//  (busca, insercao ou remocao) o no acessado e levado ate a RAIZ por meio
//  de rotacoes (operacao "splay"). Assim, elementos usados com frequencia
//  ficam perto da raiz -> otimo para padroes de acesso com localidade.
//
//  Nao guarda fator de balanceamento nem altura: o balanceamento e apenas
//  AMORTIZADO. Cada operacao custa O(log n) AMORTIZADO (nao no pior caso
//  individual, mas na media de uma sequencia de operacoes).
//
//  Casos de rotacao no splay (levando x para cima):
//    zig      : x e filho da raiz              -> 1 rotacao
//    zig-zig  : x e o pai sao filhos do mesmo lado -> 2 rotacoes iguais
//    zig-zag  : x e o pai sao filhos de lados opostos -> 2 rotacoes opostas
// ===========================================================================
#ifndef SPLAY_HPP
#define SPLAY_HPP

#include <ostream>

class Splay {
private:
    struct No {
        int chave;
        No *esq = nullptr, *dir = nullptr, *pai = nullptr;
        No(int c) : chave(c) {}
    };

    No* raiz = nullptr;

    void destruir(No* n) {
        if (!n) return;
        destruir(n->esq);
        destruir(n->dir);
        delete n;
    }

    // rotacao a esquerda em x (o filho direito sobe)
    void rotEsq(No* x) {
        No* y = x->dir;
        x->dir = y->esq;
        if (y->esq) y->esq->pai = x;
        y->pai = x->pai;
        if (!x->pai) raiz = y;
        else if (x == x->pai->esq) x->pai->esq = y;
        else x->pai->dir = y;
        y->esq = x;
        x->pai = y;
    }

    // rotacao a direita em x (o filho esquerdo sobe)
    void rotDir(No* x) {
        No* y = x->esq;
        x->esq = y->dir;
        if (y->dir) y->dir->pai = x;
        y->pai = x->pai;
        if (!x->pai) raiz = y;
        else if (x == x->pai->dir) x->pai->dir = y;
        else x->pai->esq = y;
        y->dir = x;
        x->pai = y;
    }

    // leva x ate a raiz aplicando zig / zig-zig / zig-zag
    void splay(No* x) {
        while (x->pai) {
            No* p = x->pai;
            No* g = p->pai;
            if (!g) {                                   // ZIG
                if (x == p->esq) rotDir(p); else rotEsq(p);
            } else if (x == p->esq && p == g->esq) {    // ZIG-ZIG (esq)
                rotDir(g); rotDir(p);
            } else if (x == p->dir && p == g->dir) {    // ZIG-ZIG (dir)
                rotEsq(g); rotEsq(p);
            } else if (x == p->dir && p == g->esq) {    // ZIG-ZAG
                rotEsq(p); rotDir(g);
            } else {                                    // ZIG-ZAG
                rotDir(p); rotEsq(g);
            }
        }
    }

    // busca padrao de BST; devolve o ultimo no visitado (para dar splay)
    No* buscarNo(int chave) const {
        No* atual = raiz;
        No* ultimo = nullptr;
        while (atual) {
            ultimo = atual;
            if (chave == atual->chave) return atual;
            atual = (chave < atual->chave) ? atual->esq : atual->dir;
        }
        return ultimo;
    }

    int escreverRec(No* n, std::ostream& os, int& proxId,
                    const char* lado, int idPai) const {
        int meuId = proxId++;
        os << "N " << meuId << " " << n->chave << "\n";
        if (idPai >= 0) os << "E " << idPai << " " << meuId << " " << lado << "\n";
        if (n->esq) escreverRec(n->esq, os, proxId, "L", meuId);
        if (n->dir) escreverRec(n->dir, os, proxId, "R", meuId);
        return meuId;
    }

public:
    ~Splay() { destruir(raiz); }

    // Insercao: BST comum + splay do no novo ate a raiz.
    void inserir(int chave) {
        if (!raiz) { raiz = new No(chave); return; }
        No* atual = raiz;
        No* pai = nullptr;
        while (atual) {
            pai = atual;
            if (chave == atual->chave) { splay(atual); return; } // ja existe
            atual = (chave < atual->chave) ? atual->esq : atual->dir;
        }
        No* novo = new No(chave);
        novo->pai = pai;
        if (chave < pai->chave) pai->esq = novo; else pai->dir = novo;
        splay(novo);
    }

    // Busca: devolve true/false e leva o no acessado (ou o ultimo) a raiz.
    bool buscar(int chave) {
        No* n = buscarNo(chave);
        if (!n) return false;
        splay(n);
        return n->chave == chave;
    }

    // Remocao: da splay na chave, remove a raiz e "junta" as duas subarvores.
    bool remover(int chave) {
        No* n = buscarNo(chave);
        if (!n) return false;
        splay(n);
        if (n->chave != chave) return false;   // nao existia

        No* esq = raiz->esq;
        No* dir = raiz->dir;
        if (esq) esq->pai = nullptr;
        if (dir) dir->pai = nullptr;
        delete raiz;

        if (!esq) { raiz = dir; return true; }
        // maior elemento da subarvore esquerda vira a nova raiz (sem filho dir)
        raiz = esq;
        No* maior = esq;
        while (maior->dir) maior = maior->dir;
        splay(maior);
        maior->dir = dir;
        if (dir) dir->pai = maior;
        return true;
    }

    void escreverArvore(std::ostream& os) const {
        int proxId = 0;
        if (raiz) escreverRec(raiz, os, proxId, "-", -1);
    }
};

#endif // SPLAY_HPP
