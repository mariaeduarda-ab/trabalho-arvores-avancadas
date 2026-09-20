#pragma once
#include <ostream>
#include <cmath>
#include <limits>

class KDTree {
public:
    struct Point { double x, y; };

private:
    struct Node {
        Point p;
        Node *left = nullptr, *right = nullptr;
        Node(const Point& q) : p(q) {}
    };

    Node* root = nullptr;

    static double coord(const Point& p, int axis) { return axis == 0 ? p.x : p.y; }
    static bool equal(const Point& a, const Point& b) { return a.x == b.x && a.y == b.y; }
    static double dist2(const Point& a, const Point& b) {
        double dx = a.x - b.x, dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left); destroy(n->right); delete n;
    }

    Node* insertRec(Node* n, const Point& p, int depth) {
        if (!n) return new Node(p);
        int axis = depth % 2;
        if (coord(p, axis) < coord(n->p, axis))
            n->left = insertRec(n->left, p, depth + 1);
        else
            n->right = insertRec(n->right, p, depth + 1);
        return n;
    }

    bool searchRec(Node* n, const Point& p, int depth) const {
        if (!n) return false;
        if (equal(n->p, p)) return true;
        int axis = depth % 2;
        if (coord(p, axis) < coord(n->p, axis)) return searchRec(n->left, p, depth + 1);
        return searchRec(n->right, p, depth + 1);
    }

    static Node* minNode(Node* a, Node* b, int axis) {
        if (!a) return b;
        if (!b) return a;
        return (coord(a->p, axis) <= coord(b->p, axis)) ? a : b;
    }

    Node* findMin(Node* n, int target_axis, int depth) const {
        if (!n) return nullptr;
        int axis = depth % 2;
        if (axis == target_axis) {
            if (!n->left) return n;
            return minNode(n, findMin(n->left, target_axis, depth + 1), target_axis);
        }
        Node* ml = findMin(n->left, target_axis, depth + 1);
        Node* mr = findMin(n->right, target_axis, depth + 1);
        return minNode(minNode(n, ml, target_axis), mr, target_axis);
    }

    Node* removeRec(Node* n, const Point& p, int depth, bool& found) {
        if (!n) return nullptr;
        int axis = depth % 2;
        if (equal(n->p, p)) {
            found = true;
            if (n->right) {
                Node* m = findMin(n->right, axis, depth + 1);
                n->p = m->p;
                n->right = removeRec(n->right, m->p, depth + 1, found);
            } else if (n->left) {
                Node* m = findMin(n->left, axis, depth + 1);
                n->p = m->p;
                n->right = removeRec(n->left, m->p, depth + 1, found);
                n->left = nullptr;
            } else {
                delete n;
                return nullptr;
            }
            return n;
        }
        if (coord(p, axis) < coord(n->p, axis))
            n->left = removeRec(n->left, p, depth + 1, found);
        else
            n->right = removeRec(n->right, p, depth + 1, found);
        return n;
    }

    void nearestRec(Node* n, const Point& target, int depth, Node*& best, double& best_d2) const {
        if (!n) return;
        double d2 = dist2(n->p, target);
        if (d2 < best_d2) { best_d2 = d2; best = n; }
        int axis = depth % 2;
        double diff = coord(target, axis) - coord(n->p, axis);
        Node* near_side = (diff < 0) ? n->left : n->right;
        Node* far_side = (diff < 0) ? n->right : n->left;
        nearestRec(near_side, target, depth + 1, best, best_d2);
        if (diff * diff < best_d2)
            nearestRec(far_side, target, depth + 1, best, best_d2);
    }

    int writeRec(Node* n, std::ostream& os, int& next_id,
                 const char* side, int parent_id, int depth) const {
        int my_id = next_id++;
        char axis = (depth % 2 == 0) ? 'x' : 'y';
        os << "N " << my_id << " (" << (int)n->p.x << "," << (int)n->p.y << ")[" << axis << "]\n";
        if (parent_id >= 0) os << "E " << parent_id << " " << my_id << " " << side << "\n";
        if (n->left) writeRec(n->left, os, next_id, "L", my_id, depth + 1);
        if (n->right) writeRec(n->right, os, next_id, "R", my_id, depth + 1);
        return my_id;
    }

    void writeSpaceRec(Node* n, std::ostream& os, int depth,
                       double xmin, double xmax, double ymin, double ymax) const {
        if (!n) return;
        os << "PT " << n->p.x << " " << n->p.y << "\n";
        int axis = depth % 2;
        if (axis == 0) {
            os << "SEG " << n->p.x << " " << ymin << " " << n->p.x << " " << ymax << "\n";
            writeSpaceRec(n->left, os, depth + 1, xmin, n->p.x, ymin, ymax);
            writeSpaceRec(n->right, os, depth + 1, n->p.x, xmax, ymin, ymax);
        } else {
            os << "SEG " << xmin << " " << n->p.y << " " << xmax << " " << n->p.y << "\n";
            writeSpaceRec(n->left, os, depth + 1, xmin, xmax, ymin, n->p.y);
            writeSpaceRec(n->right, os, depth + 1, xmin, xmax, n->p.y, ymax);
        }
    }

public:
    ~KDTree() { destroy(root); }

    void insert(const Point& p) { root = insertRec(root, p, 0); }
    void insert(double x, double y) { insert(Point{x, y}); }
    bool search(const Point& p) const { return searchRec(root, p, 0); }

    bool remove(const Point& p) {
        bool found = false;
        root = removeRec(root, p, 0, found);
        return found;
    }

    Point nearestNeighbor(const Point& target) const {
        Node* best = nullptr;
        double best_d2 = std::numeric_limits<double>::infinity();
        nearestRec(root, target, 0, best, best_d2);
        return best ? best->p : Point{0, 0};
    }

    void writeTree(std::ostream& os) const {
        int next_id = 0;
        if (root) writeRec(root, os, next_id, "-", -1, 0);
    }

    void writeSpace(std::ostream& os, double xmin, double xmax,
                    double ymin, double ymax) const {
        os << "BB " << xmin << " " << xmax << " " << ymin << " " << ymax << "\n";
        writeSpaceRec(root, os, 0, xmin, xmax, ymin, ymax);
    }
};
