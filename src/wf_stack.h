#pragma once
#include <atomic>

template<typename T>
class WFStack {
    struct Node {
        T value;
        Node* next;
        std::atomic<bool> marked{false};
    };

    std::atomic<Node*> head{nullptr};

public:
    void push(const T& val) {
        auto* n = new Node{val, nullptr};
        n->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(n->next, n)) { /* retry */ }
    }

    bool pop(T& out) {
        // Use relaxed memory order for head load.
        Node* curr = head.load();
        while (curr) {
            bool unmarked = false;
            if (!curr->marked.load(std::memory_order_relaxed)) {
                // Use a local variable for compare_exchange expected value.
                if (curr->marked.compare_exchange_strong(unmarked, true,
                                                std::memory_order_acq_rel,
                                                std::memory_order_relaxed)) {
                    // Successfully marked the node.
                    out = curr->value;
                    cleanup();
                    return true;
                }
            }
            curr = curr->next;
        }
        return false;
    }

private:
    void cleanup() {
        // lazily delete marked nodes (simplified)
        Node* prev = nullptr;
        Node* curr = head.load();
        while (curr) {
            if (curr->marked.load()) {
                Node* tmp = curr;
                if (prev) prev->next = curr->next;
                curr = curr->next;
                delete tmp;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }
};
