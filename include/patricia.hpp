// ===========================================================================
//  patricia.hpp  -  Arvore Patricia (Radix Tree compacta)
//  Trabalho Pratico I - Estruturas em Arvores Avancadas
//
//  A Patricia e uma Trie COMPACTADA: cada aresta guarda uma SUBSTRING
//  (varios caracteres) em vez de um unico caractere. Sequencias de nos
//  com um so filho sao "colapsadas" em uma unica aresta. Isso reduz
//  drasticamente o numero de nos quando ha longos prefixos sem bifurcacao.
//
//  Ideia central:
//   - Inserir: caminha comparando substrings; quando ha um prefixo comum
//     PARCIAL, a aresta e DIVIDIDA (split) criando um no intermediario.
//   - Remover: alem de apagar, pode ser preciso FUNDIR (merge) um no que
//     ficou com um unico filho, para manter a arvore compacta.
//
//  Cada No guarda o rotulo da aresta que vem do PAI (rotuloAresta), o que
//  facilita as operacoes de split e merge.
// ===========================================================================
#ifndef PATRICIA_HPP
#define PATRICIA_HPP

#include <map>
#include <string>
#include <vector>
#include <ostream>

class Patricia {
private:
    struct No {
        bool fim = false;               // marca fim de uma chave
        std::string rotuloAresta;       // substring da aresta pai -> este no
        std::map<char, No*> filhos;     // indexado pelo 1o caractere do rotulo
    };

    No* raiz;

    void destruir(No* n) {
        if (!n) return;
        for (auto& p : n->filhos) destruir(p.second);
        delete n;
    }

    // tamanho do prefixo comum entre a substring L e word a partir de 'ini'
    static size_t prefixoComum(const std::string& L, const std::string& word, size_t ini) {
        size_t k = 0;
        while (k < L.size() && ini + k < word.size() && L[k] == word[ini + k]) k++;
        return k;
    }

    int escreverRec(No* n, std::ostream& os, int& proxId, int idPai) const {
        int meuId = proxId++;
        os << "N " << meuId << " " << (n->fim ? "*" : "-") << "\n";
        if (idPai >= 0)
            os << "E " << idPai << " " << meuId << " " << n->rotuloAresta << "\n";
        for (auto& p : n->filhos)
            escreverRec(p.second, os, proxId, meuId);
        return meuId;
    }

public:
    Patricia() { raiz = new No(); }
    ~Patricia() { destruir(raiz); }

    // Insere 'word' criando/dividindo arestas conforme necessario.
    void inserir(const std::string& word) {
        No* atual = raiz;
        size_t i = 0;
        while (true) {
            if (i == word.size()) { atual->fim = true; return; }
            char c = word[i];
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) {
                // nao ha aresta comecando por c: cria uma aresta com o resto
                No* nova = new No();
                nova->rotuloAresta = word.substr(i);
                nova->fim = true;
                atual->filhos[c] = nova;
                return;
            }
            No* filho = it->second;
            const std::string L = filho->rotuloAresta;
            size_t k = prefixoComum(L, word, i);
            if (k == L.size()) {
                // consumiu a aresta inteira: desce e continua
                atual = filho;
                i += k;
                continue;
            }
            // prefixo comum PARCIAL -> DIVIDIR a aresta em um no intermediario 'meio'
            No* meio = new No();
            meio->rotuloAresta = L.substr(0, k);       // parte comum
            filho->rotuloAresta = L.substr(k);          // resto fica com o filho antigo
            atual->filhos[c] = meio;                    // pai passa a apontar para 'meio'
            meio->filhos[filho->rotuloAresta[0]] = filho;
            if (i + k == word.size()) {
                meio->fim = true;                       // a palavra termina no split
            } else {
                No* nova = new No();
                nova->rotuloAresta = word.substr(i + k);
                nova->fim = true;
                meio->filhos[nova->rotuloAresta[0]] = nova;
            }
            return;
        }
    }

    // Busca a chave completa seguindo arestas por substrings inteiras.
    bool buscar(const std::string& word) const {
        const No* atual = raiz;
        size_t i = 0;
        while (i < word.size()) {
            char c = word[i];
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) return false;
            const std::string& L = it->second->rotuloAresta;
            if (i + L.size() > word.size()) return false;
            if (word.compare(i, L.size(), L) != 0) return false; // aresta nao casa
            atual = it->second;
            i += L.size();
        }
        return atual->fim;
    }

    // Remove a chave e recompacta a arvore (fusao de nos com 1 filho).
    bool remover(const std::string& word) {
        std::vector<No*> caminho;
        No* atual = raiz;
        caminho.push_back(atual);
        size_t i = 0;
        while (i < word.size()) {
            char c = word[i];
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) return false;
            const std::string& L = it->second->rotuloAresta;
            if (i + L.size() > word.size() || word.compare(i, L.size(), L) != 0)
                return false;
            atual = it->second;
            caminho.push_back(atual);
            i += L.size();
        }
        if (!atual->fim) return false; // chave nao existia
        atual->fim = false;

        // recompactacao de baixo para cima ao longo do caminho
        for (size_t idx = caminho.size() - 1; idx >= 1; --idx) {
            No* n = caminho[idx];
            No* pai = caminho[idx - 1];
            if (n->filhos.empty() && !n->fim) {
                // folha inutil: remove do pai
                pai->filhos.erase(n->rotuloAresta[0]);
                delete n;
                // continua subindo: o pai pode ter ficado com 1 filho
            } else if (n->filhos.size() == 1 && !n->fim) {
                // no com 1 filho e sem fim: funde o filho nele
                No* f = n->filhos.begin()->second;
                n->rotuloAresta += f->rotuloAresta;
                n->fim = f->fim;
                n->filhos = f->filhos;
                delete f;
                break; // ancestrais nao mudam estruturalmente
            } else {
                break; // no permanece; nada a fazer acima
            }
        }
        return true;
    }

    void escreverArvore(std::ostream& os) const {
        int proxId = 0;
        escreverRec(raiz, os, proxId, -1);
    }
};

#endif // PATRICIA_HPP
