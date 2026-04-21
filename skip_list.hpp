#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include <vector>
#include <random>
#include <optional>
#include <cstdlib>
#include <ctime>

// Generic skip list supporting insert/search/delete for type T.
// Requires only operator< on T (strict weak ordering).
// Equality: !(a<b) && !(b<a)

template<typename T>
class SkipList {
private:
    struct Node {
        std::optional<T> value; // header has no value
        std::vector<Node*> forward; // forward pointers per level
        explicit Node(int lvl) : value(std::nullopt), forward(lvl, nullptr) {}
        Node(int lvl, const T& val) : value(val), forward(lvl, nullptr) {}
    };

    int maxLevel;        // maximum allowed levels
    double p;            // probability for level promotion
    int level;           // current highest level
    Node* header;        // header node with maxLevel forward pointers (no value)

    // RNG for random level
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> dist;

    int randomLevel() {
        int lvl = 1;
        while (dist(rng) < p && lvl < maxLevel) ++lvl;
        return lvl;
    }

    // Disable copy
    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

public:
    SkipList()
        : maxLevel(32), p(0.5), level(1), header(nullptr),
          rng(std::mt19937_64(std::random_device{}())), dist(0.0, 1.0) {
        header = new Node(maxLevel); // no T construction
    }

    ~SkipList() {
        Node* cur = header;
        while (cur) {
            Node* next = cur->forward[0];
            delete cur;
            cur = next;
        }
    }

    // Insert a value into the skip list. If the value already exists, do nothing.
    void insert(const T & item) {
        std::vector<Node*> update(maxLevel, nullptr);
        Node* x = header;
        for (int i = level - 1; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value.has_value() && x->forward[i]->value.value() < item) {
                x = x->forward[i];
            }
            update[i] = x;
        }
        x = x->forward[0];
        if (x && x->value.has_value()) {
            const T& v = x->value.value();
            if (!(item < v) && !(v < item)) {
                return; // already exists
            }
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

    // Search for a value in the skip list
    bool search(const T & item) {
        Node* x = header;
        for (int i = level - 1; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value.has_value() && x->forward[i]->value.value() < item) {
                x = x->forward[i];
            }
        }
        x = x->forward[0];
        if (x && x->value.has_value()) {
            const T& v = x->value.value();
            return !(item < v) && !(v < item);
        }
        return false;
    }

    // Delete a value from the skip list. If the value does not exist, do nothing.
    void deleteItem(const T & item) {
        std::vector<Node*> update(maxLevel, nullptr);
        Node* x = header;
        for (int i = level - 1; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value.has_value() && x->forward[i]->value.value() < item) {
                x = x->forward[i];
            }
            update[i] = x;
        }
        x = x->forward[0];
        if (!(x && x->value.has_value())) return; // not found
        const T& v = x->value.value();
        if ((item < v) || (v < item)) return; // not found
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
