// ===========================================================================
//  trie.hpp  -  Trie (Arvore de Prefixos)
//  Trabalho Pratico I - Estruturas em Arvores Avancadas
//
//  A Trie e uma arvore n-aria onde CADA ARESTA representa um caractere.
//  Uma palavra e o caminho da raiz ate um no marcado como fim de palavra.
//  Prefixos comuns compartilham o mesmo caminho, o que economiza comparacoes.
//
//  Operacoes: inserir, buscar, remover, comecaCom (prefixo).
//  Custo das operacoes: O(m), onde m = comprimento da chave (NAO depende de n).
// ===========================================================================
#ifndef TRIE_HPP
#define TRIE_HPP

#include <map>
#include <string>
#include <ostream>

class Trie {
private:
    struct No {
        bool fimDePalavra = false;      // marca se um caminho termina aqui
        std::map<char, No*> filhos;     // map ordenado -> saida alfabetica bonita
    };

    No* raiz;

    // libera memoria recursivamente
    void destruir(No* n) {
        if (!n) return;
        for (auto& par : n->filhos) destruir(par.second);
        delete n;
    }

    // usado na remocao: apaga um no se ele ficou "inutil"
    // (nao e fim de palavra e nao tem filhos)
    bool removerRec(No* n, const std::string& chave, size_t i) {
        if (i == chave.size()) {
            if (!n->fimDePalavra) return false; // palavra nao existia
            n->fimDePalavra = false;
            return n->filhos.empty();           // pode ser apagada pelo pai?
        }
        char c = chave[i];
        auto it = n->filhos.find(c);
        if (it == n->filhos.end()) return false; // caminho nao existe
        bool apagarFilho = removerRec(it->second, chave, i + 1);
        if (apagarFilho) {
            delete it->second;
            n->filhos.erase(it);
            // este no tambem pode ser apagado se ficou vazio e nao termina palavra
            return n->filhos.empty() && !n->fimDePalavra;
        }
        return false;
    }

    // percorre a arvore atribuindo um id a cada no e imprime no formato .tree
    int escreverRec(No* n, std::ostream& os, int& proxId,
                    const std::string& rotuloAresta, int idPai) const {
        int meuId = proxId++;
        std::string rotuloNo = n->fimDePalavra ? "*" : "";  // '*' = fim de palavra
        os << "N " << meuId << " " << (rotuloNo.empty() ? "-" : rotuloNo) << "\n";
        if (idPai >= 0)
            os << "E " << idPai << " " << meuId << " " << rotuloAresta << "\n";
        for (auto& par : n->filhos)
            escreverRec(par.second, os, proxId, std::string(1, par.first), meuId);
        return meuId;
    }

public:
    Trie() { raiz = new No(); }
    ~Trie() { destruir(raiz); }

    // Insere a chave caractere a caractere, criando nos quando necessario.
    void inserir(const std::string& chave) {
        No* atual = raiz;
        for (char c : chave) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end())
                it = atual->filhos.emplace(c, new No()).first;
            atual = it->second;
        }
        atual->fimDePalavra = true;
    }

    // Retorna true se a palavra completa existe.
    bool buscar(const std::string& chave) const {
        const No* atual = raiz;
        for (char c : chave) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) return false;
            atual = it->second;
        }
        return atual->fimDePalavra;
    }

    // Retorna true se algum caminho comeca com o prefixo dado.
    bool comecaCom(const std::string& prefixo) const {
        const No* atual = raiz;
        for (char c : prefixo) {
            auto it = atual->filhos.find(c);
            if (it == atual->filhos.end()) return false;
            atual = it->second;
        }
        return true;
    }

    // Remove a chave; retorna true se a palavra existia.
    bool remover(const std::string& chave) {
        if (!buscar(chave)) return false;
        removerRec(raiz, chave, 0);
        return true;
    }

    // Exporta a estrutura no formato lido pelo tools/treeviz.py
    void escreverArvore(std::ostream& os) const {
        int proxId = 0;
        escreverRec(raiz, os, proxId, "-", -1);
    }
};

#endif // TRIE_HPP
