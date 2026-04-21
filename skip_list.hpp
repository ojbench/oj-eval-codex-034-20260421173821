#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include <iostream>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <vector>
#include <random>

// Generic skip list supporting insert/search/delete for type T.
// Requires only operator< on T (strict weak ordering).
// Equality: !(a<b) && !(b<a)

template<typename T>
class SkipList {
private:
    struct Node {
        T value;
        std::vector<Node*> forward;
        Node(int lvl, const T& val): value(val), forward(lvl, nullptr) {}
    };

    int maxLevel;
    double p;
    int level;
    Node* header;

    std::mt19937_64 rng;
    std::uniform_real_distribution<double> dist;

    int randomLevel() {
        int lvl = 1;
        while (dist(rng) < p && lvl < maxLevel) ++lvl;
        return lvl;
    }

    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

public:
    SkipList()
        : maxLevel(32), p(0.5), level(1), header(nullptr),
          rng(std::mt19937_64(std::random_device{}())), dist(0.0, 1.0) {
        header = new Node(maxLevel, T{});
    }

    ~SkipList() {
        Node* cur = header;
        while (cur) {
            Node* next = cur->forward[0];
            delete cur;
            cur = next;
        }
    }

    void insert(const T & item) {
        std::vector<Node*> update(maxLevel, nullptr);
        Node* x = header;
        for (int i = level - 1; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value < item) {
                x = x->forward[i];
            }
            update[i] = x;
        }
        x = x->forward[0];
        if (x && !(item < x->value) && !(x->value < item)) {
            return; // already exists
        }
        int rl = randomLevel();
        if (rl > level) {
            for (int i = level; i < rl; ++i) update[i] = header;
            level = rl;
        }
        Node* newNode = new Node(rl, item);
        for (int i = 0; i < rl; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    bool search(const T & item) {
        Node* x = header;
        for (int i = level - 1; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value < item) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        return x && !(item < x->value) && !(x->value < item);
    }

    void deleteItem(const T & item) {
        std::vector<Node*> update(maxLevel, nullptr);
        Node* x = header;
        for (int i = level - 1; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value < item) {
                x = x->forward[i];
            }
            update[i] = x;
        }
        x = x->forward[0];
        if (!x || (item < x->value) || (x->value < item)) {
            return; // not found
        }
        int rl = static_cast<int>(x->forward.size());
        for (int i = 0; i < rl; ++i) {
            if (update[i]->forward[i] == x) {
                update[i]->forward[i] = x->forward[i];
            }
        }
        delete x;
        while (level > 1 && header->forward[level - 1] == nullptr) {
            --level;
        }
    }
};

#endif
