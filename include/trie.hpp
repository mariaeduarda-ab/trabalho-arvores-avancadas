#pragma once
#include <map>
#include <string>
#include <ostream>

class Trie {
private:
    struct Node {
        bool is_end = false;
        std::map<char, Node*> children;
    };

    Node* root;

    void destroy(Node* n) {
        if (!n) return;
        for (auto& entry : n->children) destroy(entry.second);
        delete n;
    }

    bool removeRec(Node* n, const std::string& key, size_t i) {
        if (i == key.size()) {
            if (!n->is_end) return false;
            n->is_end = false;
            return n->children.empty();
        }
        char c = key[i];
        auto it = n->children.find(c);
        if (it == n->children.end()) return false;
        bool drop_child = removeRec(it->second, key, i + 1);
        if (drop_child) {
            delete it->second;
            n->children.erase(it);
            return n->children.empty() && !n->is_end;
        }
        return false;
    }

    int writeRec(Node* n, std::ostream& os, int& next_id,
                 const std::string& edge_label, int parent_id) const {
        int my_id = next_id++;
        std::string label = n->is_end ? "*" : "";
        os << "N " << my_id << " " << (label.empty() ? "-" : label) << "\n";
        if (parent_id >= 0)
            os << "E " << parent_id << " " << my_id << " " << edge_label << "\n";
        for (auto& entry : n->children)
            writeRec(entry.second, os, next_id, std::string(1, entry.first), my_id);
        return my_id;
    }

public:
    Trie() { root = new Node(); }
    ~Trie() { destroy(root); }

    void insert(const std::string& key) {
        Node* cur = root;
        for (char c : key) {
            auto it = cur->children.find(c);
            if (it == cur->children.end())
                it = cur->children.emplace(c, new Node()).first;
            cur = it->second;
        }
        cur->is_end = true;
    }

    bool search(const std::string& key) const {
        const Node* cur = root;
        for (char c : key) {
            auto it = cur->children.find(c);
            if (it == cur->children.end()) return false;
            cur = it->second;
        }
        return cur->is_end;
    }

    bool startsWith(const std::string& prefix) const {
        const Node* cur = root;
        for (char c : prefix) {
            auto it = cur->children.find(c);
            if (it == cur->children.end()) return false;
            cur = it->second;
        }
        return true;
    }

    bool remove(const std::string& key) {
        if (!search(key)) return false;
        removeRec(root, key, 0);
        return true;
    }

    void writeTree(std::ostream& os) const {
        int next_id = 0;
        writeRec(root, os, next_id, "-", -1);
    }
};
