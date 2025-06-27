#pragma once
#include <atomic>
#include <vector>
#include <optional>
#include <cstdint>
#include <thread>
#include <cassert>

template<typename T>
class WCQ {
    struct Entry {
        std::atomic<uint64_t> cycle{};
        std::atomic<T*> ptr{};
    };

    struct ThreadRec {
        uint64_t initTail, localTail;
        uint64_t initHead, localHead;
        size_t index;
        bool isEnq;
        bool pending = false;
        uint64_t seq1 = 0, seq2 = 0;
    };

    std::vector<Entry> buffer;
    std::vector<ThreadRec> recs;
    const size_t capacity;
    std::atomic<uint64_t> enq_ticket{0}, deq_ticket{0};
    std::atomic<int64_t> threshold{-1};

public:
    WCQ(size_t cap, size_t numThreads)
      : buffer(cap), recs(numThreads), capacity(cap) {}

    bool enqueue(T *item, int tid) {
        help_threads(tid);
        // Fast path (SCQ-like)
        for(int i = 0; i < MAX_PATIENCE; ++i) {
            uint64_t t = enq_ticket.fetch_add(1, std::memory_order_relaxed);
            auto &e = buffer[t % capacity];
            uint64_t c = t / capacity;
            if (e.cycle.load() == c && e.ptr.load() == nullptr) {
                T* expected = nullptr;
                if (e.ptr.compare_exchange_strong(expected, item,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                    threshold.store(std::numeric_limits<int64_t>::max());
                    return true;
                }
            }
        }
        // Slow path
        auto &r = recs[tid];
        r.initTail = r.localTail = enq_ticket.fetch_add(0);
        r.index = uintptr_t(item);
        r.isEnq = true;
        r.seq2 = r.seq1;
        r.pending = true;
        enqueue_slow(tid);
        r.pending = false;
        r.seq1++;
        return true;
    }

    std::optional<T*> dequeue(int tid) {
        help_threads(tid);
        // Fast path
        for(int i = 0; i < MAX_PATIENCE; ++i) {
            uint64_t t = deq_ticket.fetch_add(1, std::memory_order_relaxed);
            auto &e = buffer[t % capacity];
            uint64_t c = t / capacity;
            T* ptr = e.ptr.load();
            if (ptr && e.cycle.load() == c)
                if (e.ptr.compare_exchange_strong(ptr, nullptr))
                    return ptr;
        }
        // Slow path
        auto &r = recs[tid];
        r.initHead = r.localHead = deq_ticket.fetch_add(0);
        r.isEnq = false;
        r.seq2 = r.seq1;
        r.pending = true;
        dequeue_slow(tid);
        r.pending = false;
        r.seq1++;
        return std::nullopt; // For demo
    }

private:
    void help_threads(int tid) {
        size_t n = recs.size();
        for(size_t i = 0; i < n; ++i) {
            size_t j = (tid + i) % n;
            auto &r = recs[j];
            if (r.pending) {
                if (r.isEnq) enqueue_slow(j);
                else dequeue_slow(j);
            }
        }
    }

    void enqueue_slow(int tid) {
        auto &r = recs[tid];
        [[maybe_unused]] uint64_t t = r.localTail;
        // Helping fast path eventually succeeds
        for (;;) {
            // try SCQ fast path on behalf of enqueuer
            uint64_t vals = enq_ticket.fetch_add(1);
            auto &e = buffer[vals % capacity];
            //if (e.ptr.compare_exchange_strong(nullptr, (T*)r.index)) break;
            T* expected = nullptr;
            if (e.ptr.compare_exchange_strong(expected, (T*)r.index,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                        break;
            }
            threshold--;
        }
    }

    void dequeue_slow(int tid) {
        auto &r = recs[tid];
        [[maybe_unused]] uint64_t t = r.localTail;        
        for (;;) {
            uint64_t vals = deq_ticket.fetch_add(1);
            auto &e = buffer[vals % capacity];
            T* ptr = e.ptr.load();
            if (ptr && e.ptr.compare_exchange_strong(ptr, nullptr)) break;
            threshold--;
        }
    }

    static constexpr int MAX_PATIENCE = 64;
};
// Note: This is a simplified version of the WCQ queue. In a real-world scenario, you would implement
// proper memory management, error handling, and possibly more complex logic for thread management.