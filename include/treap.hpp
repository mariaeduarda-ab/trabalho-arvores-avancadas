#pragma once
#include <ostream>
#include <random>

class Treap {
private:
    struct Node {
        int key;
        int priority;
        Node *left = nullptr, *right = nullptr;
        Node(int k, int p) : key(k), priority(p) {}
    };

    Node* root = nullptr;
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist{1, 999};

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        return y;
    }

    Node* insertRec(Node* n, int key) {
        if (!n) return new Node(key, dist(rng));
        if (key < n->key) {
            n->left = insertRec(n->left, key);
            if (n->left->priority > n->priority) n = rotateRight(n);
        } else if (key > n->key) {
            n->right = insertRec(n->right, key);
            if (n->right->priority > n->priority) n = rotateLeft(n);
        }
        return n;
    }

    Node* removeRec(Node* n, int key, bool& found) {
        if (!n) return nullptr;
        if (key < n->key) {
            n->left = removeRec(n->left, key, found);
        } else if (key > n->key) {
            n->right = removeRec(n->right, key, found);
        } else {
            found = true;
            if (!n->left) { Node* t = n->right; delete n; return t; }
            if (!n->right) { Node* t = n->left; delete n; return t; }
            if (n->left->priority > n->right->priority) {
                n = rotateRight(n);
                n->right = removeRec(n->right, key, found);
            } else {
                n = rotateLeft(n);
                n->left = removeRec(n->left, key, found);
            }
        }
        return n;
    }

    bool searchRec(Node* n, int key) const {
        while (n) {
            if (key == n->key) return true;
            n = (key < n->key) ? n->left : n->right;
        }
        return false;
    }

    int writeRec(Node* n, std::ostream& os, int& next_id,
                 const char* side, int parent_id) const {
        int my_id = next_id++;
        os << "N " << my_id << " " << n->key << "(p" << n->priority << ")\n";
        if (parent_id >= 0) os << "E " << parent_id << " " << my_id << " " << side << "\n";
        if (n->left) writeRec(n->left, os, next_id, "L", my_id);
        if (n->right) writeRec(n->right, os, next_id, "R", my_id);
        return my_id;
    }

public:
    explicit Treap(unsigned seed = 42) : rng(seed) {}
    ~Treap() { destroy(root); }

    void insert(int key) { root = insertRec(root, key); }
    bool search(int key) const { return searchRec(root, key); }
    bool remove(int key) {
        bool found = false;
        root = removeRec(root, key, found);
        return found;
    }

    void writeTree(std::ostream& os) const {
        int next_id = 0;
        if (root) writeRec(root, os, next_id, "-", -1);
    }
};
