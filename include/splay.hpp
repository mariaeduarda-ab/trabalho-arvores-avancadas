#pragma once
#include <ostream>

class Splay {
private:
    struct Node {
        int key;
        Node *left = nullptr, *right = nullptr, *parent = nullptr;
        Node(int k) : key(k) {}
    };

    Node* root = nullptr;

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    void rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rotateRight(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right) y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    void splay(Node* x) {
        while (x->parent) {
            Node* p = x->parent;
            Node* g = p->parent;
            if (!g) {
                if (x == p->left) rotateRight(p); else rotateLeft(p);
            } else if (x == p->left && p == g->left) {
                rotateRight(g); rotateRight(p);
            } else if (x == p->right && p == g->right) {
                rotateLeft(g); rotateLeft(p);
            } else if (x == p->right && p == g->left) {
                rotateLeft(p); rotateRight(g);
            } else {
                rotateRight(p); rotateLeft(g);
            }
        }
    }

    Node* searchNode(int key) const {
        Node* cur = root;
        Node* last = nullptr;
        while (cur) {
            last = cur;
            if (key == cur->key) return cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return last;
    }

    int writeRec(Node* n, std::ostream& os, int& next_id,
                 const char* side, int parent_id) const {
        int my_id = next_id++;
        os << "N " << my_id << " " << n->key << "\n";
        if (parent_id >= 0) os << "E " << parent_id << " " << my_id << " " << side << "\n";
        if (n->left) writeRec(n->left, os, next_id, "L", my_id);
        if (n->right) writeRec(n->right, os, next_id, "R", my_id);
        return my_id;
    }

public:
    ~Splay() { destroy(root); }

    void insert(int key) {
        if (!root) { root = new Node(key); return; }
        Node* cur = root;
        Node* parent = nullptr;
        while (cur) {
            parent = cur;
            if (key == cur->key) { splay(cur); return; }
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        Node* fresh = new Node(key);
        fresh->parent = parent;
        if (key < parent->key) parent->left = fresh; else parent->right = fresh;
        splay(fresh);
    }

    bool search(int key) {
        Node* n = searchNode(key);
        if (!n) return false;
        splay(n);
        return n->key == key;
    }

    bool remove(int key) {
        Node* n = searchNode(key);
        if (!n) return false;
        splay(n);
        if (n->key != key) return false;

        Node* left = root->left;
        Node* right = root->right;
        if (left) left->parent = nullptr;
        if (right) right->parent = nullptr;
        delete root;

        if (!left) { root = right; return true; }
        root = left;
        Node* biggest = left;
        while (biggest->right) biggest = biggest->right;
        splay(biggest);
        biggest->right = right;
        if (right) right->parent = biggest;
        return true;
    }

    void writeTree(std::ostream& os) const {
        int next_id = 0;
        if (root) writeRec(root, os, next_id, "-", -1);
    }
};
