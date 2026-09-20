#pragma once
#include <map>
#include <string>
#include <vector>
#include <ostream>

class Patricia {
private:
    struct Node {
        bool is_end = false;
        std::string edge_label;
        std::map<char, Node*> children;
    };

    Node* root;

    void destroy(Node* n) {
        if (!n) return;
        for (auto& entry : n->children) destroy(entry.second);
        delete n;
    }

    static size_t commonPrefix(const std::string& label, const std::string& word, size_t start) {
        size_t k = 0;
        while (k < label.size() && start + k < word.size() && label[k] == word[start + k]) k++;
        return k;
    }

    int writeRec(Node* n, std::ostream& os, int& next_id, int parent_id) const {
        int my_id = next_id++;
        os << "N " << my_id << " " << (n->is_end ? "*" : "-") << "\n";
        if (parent_id >= 0)
            os << "E " << parent_id << " " << my_id << " " << n->edge_label << "\n";
        for (auto& entry : n->children)
            writeRec(entry.second, os, next_id, my_id);
        return my_id;
    }

public:
    Patricia() { root = new Node(); }
    ~Patricia() { destroy(root); }

    void insert(const std::string& word) {
        Node* cur = root;
        size_t i = 0;
        while (true) {
            if (i == word.size()) { cur->is_end = true; return; }
            char c = word[i];
            auto it = cur->children.find(c);
            if (it == cur->children.end()) {
                Node* fresh = new Node();
                fresh->edge_label = word.substr(i);
                fresh->is_end = true;
                cur->children[c] = fresh;
                return;
            }
            Node* child = it->second;
            const std::string label = child->edge_label;
            size_t k = commonPrefix(label, word, i);
            if (k == label.size()) {
                cur = child;
                i += k;
                continue;
            }
            Node* mid = new Node();
            mid->edge_label = label.substr(0, k);
            child->edge_label = label.substr(k);
            cur->children[c] = mid;
            mid->children[child->edge_label[0]] = child;
            if (i + k == word.size()) {
                mid->is_end = true;
            } else {
                Node* fresh = new Node();
                fresh->edge_label = word.substr(i + k);
                fresh->is_end = true;
                mid->children[fresh->edge_label[0]] = fresh;
            }
            return;
        }
    }

    bool search(const std::string& word) const {
        const Node* cur = root;
        size_t i = 0;
        while (i < word.size()) {
            char c = word[i];
            auto it = cur->children.find(c);
            if (it == cur->children.end()) return false;
            const std::string& label = it->second->edge_label;
            if (i + label.size() > word.size()) return false;
            if (word.compare(i, label.size(), label) != 0) return false;
            cur = it->second;
            i += label.size();
        }
        return cur->is_end;
    }

    bool remove(const std::string& word) {
        std::vector<Node*> path;
        Node* cur = root;
        path.push_back(cur);
        size_t i = 0;
        while (i < word.size()) {
            char c = word[i];
            auto it = cur->children.find(c);
            if (it == cur->children.end()) return false;
            const std::string& label = it->second->edge_label;
            if (i + label.size() > word.size() || word.compare(i, label.size(), label) != 0)
                return false;
            cur = it->second;
            path.push_back(cur);
            i += label.size();
        }
        if (!cur->is_end) return false;
        cur->is_end = false;

        for (size_t idx = path.size() - 1; idx >= 1; --idx) {
            Node* n = path[idx];
            Node* parent = path[idx - 1];
            if (n->children.empty() && !n->is_end) {
                parent->children.erase(n->edge_label[0]);
                delete n;
            } else if (n->children.size() == 1 && !n->is_end) {
                Node* child = n->children.begin()->second;
                n->edge_label += child->edge_label;
                n->is_end = child->is_end;
                n->children = child->children;
                delete child;
                break;
            } else {
                break;
            }
        }
        return true;
    }

    void writeTree(std::ostream& os) const {
        int next_id = 0;
        writeRec(root, os, next_id, -1);
    }
};
