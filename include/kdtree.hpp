// ===========================================================================
//  kdtree.hpp  -  KD-Tree (k-dimensional Tree, aqui com k = 2)
//  Trabalho Pratico I - Estruturas em Arvores Avancadas
//
//  A KD-Tree organiza PONTOS em um espaco multidimensional. Cada nivel da
//  arvore divide o espaco por uma dimensao diferente, de forma alternada:
//    profundidade par  -> compara a coordenada X (corte vertical)
//    profundidade impar-> compara a coordenada Y (corte horizontal)
//
//  Assim o plano vai sendo PARTICIONADO em retangulos. Isso permite buscas
//  espaciais eficientes, como "vizinho mais proximo" (nearest neighbor),
//  aproveitando a poda de regioes que nao podem conter a resposta.
//
//  Operacoes: inserir, buscar (ponto exato), vizinhoMaisProximo, remover.
//  A remocao usa a tecnica classica de substituir o no removido pelo
//  MINIMO da subarvore na dimensao do corte (findMin).
// ===========================================================================
#ifndef KDTREE_HPP
#define KDTREE_HPP

#include <ostream>
#include <cmath>
#include <limits>

class KDTree {
public:
    struct Ponto { double x, y; };

private:
    struct No {
        Ponto p;
        No *esq = nullptr, *dir = nullptr;
        No(const Ponto& q) : p(q) {}
    };

    No* raiz = nullptr;

    static double coord(const Ponto& p, int eixo) { return eixo == 0 ? p.x : p.y; }
    static bool igual(const Ponto& a, const Ponto& b) { return a.x == b.x && a.y == b.y; }
    static double dist2(const Ponto& a, const Ponto& b) {
        double dx = a.x - b.x, dy = a.y - b.y;
        return dx * dx + dy * dy;                 // distancia ao quadrado (evita sqrt)
    }

    void destruir(No* n) {
        if (!n) return;
        destruir(n->esq); destruir(n->dir); delete n;
    }

    No* inserirRec(No* n, const Ponto& p, int prof) {
        if (!n) return new No(p);
        int eixo = prof % 2;
        if (coord(p, eixo) < coord(n->p, eixo))
            n->esq = inserirRec(n->esq, p, prof + 1);
        else
            n->dir = inserirRec(n->dir, p, prof + 1);
        return n;
    }

    bool buscarRec(No* n, const Ponto& p, int prof) const {
        if (!n) return false;
        if (igual(n->p, p)) return true;
        int eixo = prof % 2;
        if (coord(p, eixo) < coord(n->p, eixo)) return buscarRec(n->esq, p, prof + 1);
        return buscarRec(n->dir, p, prof + 1);
    }

    // no com menor coordenada 'eixoAlvo' na subarvore de 'n'
    No* findMin(No* n, int eixoAlvo, int prof) const {
        if (!n) return nullptr;
        int eixo = prof % 2;
        if (eixo == eixoAlvo) {
            if (!n->esq) return n;                 // menor esta a esquerda ou e o proprio
            return menorNo(n, findMin(n->esq, eixoAlvo, prof + 1), eixoAlvo);
        }
        // eixo diferente: o minimo pode estar dos dois lados
        No* me = findMin(n->esq, eixoAlvo, prof + 1);
        No* md = findMin(n->dir, eixoAlvo, prof + 1);
        return menorNo(menorNo(n, me, eixoAlvo), md, eixoAlvo);
    }

    static No* menorNo(No* a, No* b, int eixo) {
        if (!a) return b;
        if (!b) return a;
        return (coord(a->p, eixo) <= coord(b->p, eixo)) ? a : b;
    }

    No* removerRec(No* n, const Ponto& p, int prof, bool& achou) {
        if (!n) return nullptr;
        int eixo = prof % 2;
        if (igual(n->p, p)) {
            achou = true;
            if (n->dir) {
                // substitui pelo minimo (no eixo atual) da subarvore direita
                No* min = findMin(n->dir, eixo, prof + 1);
                n->p = min->p;
                n->dir = removerRec(n->dir, min->p, prof + 1, achou);
            } else if (n->esq) {
                // sem filho direito: usa o minimo da esquerda e move esq -> dir
                No* min = findMin(n->esq, eixo, prof + 1);
                n->p = min->p;
                n->dir = removerRec(n->esq, min->p, prof + 1, achou);
                n->esq = nullptr;
            } else {
                delete n;                          // folha
                return nullptr;
            }
            return n;
        }
        if (coord(p, eixo) < coord(n->p, eixo))
            n->esq = removerRec(n->esq, p, prof + 1, achou);
        else
            n->dir = removerRec(n->dir, p, prof + 1, achou);
        return n;
    }

    void vmpRec(No* n, const Ponto& alvo, int prof, No*& melhor, double& melhorD2) const {
        if (!n) return;
        double d2 = dist2(n->p, alvo);
        if (d2 < melhorD2) { melhorD2 = d2; melhor = n; }
        int eixo = prof % 2;
        double diff = coord(alvo, eixo) - coord(n->p, eixo);
        No* perto = (diff < 0) ? n->esq : n->dir;   // lado que contem o alvo
        No* longe = (diff < 0) ? n->dir : n->esq;
        vmpRec(perto, alvo, prof + 1, melhor, melhorD2);
        // so visita o outro lado se ele PODE conter algo mais proximo (poda)
        if (diff * diff < melhorD2)
            vmpRec(longe, alvo, prof + 1, melhor, melhorD2);
    }

    int escreverRec(No* n, std::ostream& os, int& proxId,
                    const char* lado, int idPai, int prof) const {
        int meuId = proxId++;
        char eixo = (prof % 2 == 0) ? 'x' : 'y';    // dimensao de corte deste nivel
        os << "N " << meuId << " (" << (int)n->p.x << "," << (int)n->p.y << ")[" << eixo << "]\n";
        if (idPai >= 0) os << "E " << idPai << " " << meuId << " " << lado << "\n";
        if (n->esq) escreverRec(n->esq, os, proxId, "L", meuId, prof + 1);
        if (n->dir) escreverRec(n->dir, os, proxId, "R", meuId, prof + 1);
        return meuId;
    }

    void escreverEspacoRec(No* n, std::ostream& os, int prof,
                           double xmin, double xmax, double ymin, double ymax) const {
        if (!n) return;
        os << "PT " << n->p.x << " " << n->p.y << "\n";
        int eixo = prof % 2;
        if (eixo == 0) {                            // corte vertical em x
            os << "SEG " << n->p.x << " " << ymin << " " << n->p.x << " " << ymax << "\n";
            escreverEspacoRec(n->esq, os, prof + 1, xmin, n->p.x, ymin, ymax);
            escreverEspacoRec(n->dir, os, prof + 1, n->p.x, xmax, ymin, ymax);
        } else {                                    // corte horizontal em y
            os << "SEG " << xmin << " " << n->p.y << " " << xmax << " " << n->p.y << "\n";
            escreverEspacoRec(n->esq, os, prof + 1, xmin, xmax, ymin, n->p.y);
            escreverEspacoRec(n->dir, os, prof + 1, xmin, xmax, n->p.y, ymax);
        }
    }

public:
    ~KDTree() { destruir(raiz); }

    void inserir(const Ponto& p) { raiz = inserirRec(raiz, p, 0); }
    void inserir(double x, double y) { inserir(Ponto{x, y}); }
    bool buscar(const Ponto& p) const { return buscarRec(raiz, p, 0); }

    bool remover(const Ponto& p) {
        bool achou = false;
        raiz = removerRec(raiz, p, 0, achou);
        return achou;
    }

    // devolve o ponto mais proximo do alvo (assume arvore nao vazia)
    Ponto vizinhoMaisProximo(const Ponto& alvo) const {
        No* melhor = nullptr;
        double melhorD2 = std::numeric_limits<double>::infinity();
        vmpRec(raiz, alvo, 0, melhor, melhorD2);
        return melhor ? melhor->p : Ponto{0, 0};
    }

    void escreverArvore(std::ostream& os) const {
        int proxId = 0;
        if (raiz) escreverRec(raiz, os, proxId, "-", -1, 0);
    }

    // exporta pontos + segmentos de corte para desenhar o particionamento do plano
    void escreverEspaco(std::ostream& os, double xmin, double xmax,
                        double ymin, double ymax) const {
        os << "BB " << xmin << " " << xmax << " " << ymin << " " << ymax << "\n";
        escreverEspacoRec(raiz, os, 0, xmin, xmax, ymin, ymax);
    }
};

#endif // KDTREE_HPP
