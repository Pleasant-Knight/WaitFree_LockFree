#include "wf_stack.h"
#include "wCQ_queue.h"
#include "ms_queue_wrapper.h"
#include <thread>
#include <vector>
#include <iostream>

// -------------- WFStack Demo ----------------
void demo_wfstack(int numThreads, int opsPerThread) {
    WFStack<int> st;
    auto worker = [&](int tid){
        for(int i = 0; i < opsPerThread; ++i) {
            st.push(i + tid * opsPerThread);
            int out;
            st.pop(out);
        }
    };
    std::vector<std::thread> ts;
    auto t0 = std::chrono::steady_clock::now();
    for(int i = 0; i < numThreads; ++i)
        ts.emplace_back(worker, i);
    for(auto &t : ts) t.join();
    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::cout << "[WFStack] Threads=" << numThreads
              << " Ops/thread=" << opsPerThread
              << " Throughput=" << (numThreads * opsPerThread * 2) / secs
              << " ops/s\n";
}

// ---------------- wCQ Demo ------------------
void demo_wcq(int numThreads, int opsPerThread) {
    WCQ<int> q(1024, numThreads);
    auto worker = [&](int tid){
        for(int i = 0; i < opsPerThread; ++i) {
            int *in = new int(i + tid * opsPerThread);
            q.enqueue(in, tid);
            auto out = q.dequeue(tid);
            if (out) delete *out;
        }
    };
    std::vector<std::thread> ts;
    auto t0 = std::chrono::steady_clock::now();
    for(int i = 0; i < numThreads; ++i)
        ts.emplace_back(worker, i);
    for(auto &t : ts) t.join();
    double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::cout << "[wCQ   ] Threads=" << numThreads
              << " Ops/thread=" << opsPerThread
              << " Throughput=" << (numThreads * opsPerThread * 2) / secs
              << " ops/s\n";
}

// -------------- MSQueue Demo ----------------
void demo_msqueue(int numThreads, int opsPerThread) {
    cds::Initialize();  // Initialize libcds
    {
        MSQueue<int> q;
        auto worker = [&](int tid){
            for(int i = 0; i < opsPerThread; ++i) {
                q.enqueue(i + tid * opsPerThread);
                int out;
                q.dequeue(out);
            }
        };
        std::vector<std::thread> ts;
        auto t0 = std::chrono::steady_clock::now();
        for(int i = 0; i < numThreads; ++i)
            ts.emplace_back(worker, i);
        for(auto &t : ts) t.join();
        double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        std::cout << "[MSQueue] Threads=" << numThreads
                  << " Ops/thread=" << opsPerThread
                  << " Throughput=" << (numThreads * opsPerThread * 2) / secs
                  << " ops/s\n";
    }
    cds::Terminate();
}

// Note: This main function is a simple demo to show how to use the WFStack, WCQ, and MSQueue.
// In a real-world application, you would likely have more complex logic for managing threads,
// handling errors, and possibly more sophisticated data structures or algorithms.

int main() {
/*    WFStack<int> st;
    WCQ<int> q(1024, 4); // 4 threads
    MSQueue<int> msq; // Using libcds MSQueue

    const int N = 100000;
    std::vector<std::thread> producers, consumers;

    // push/pop with WFStack...
    // enqueue/dequeue with W(CQ) and MSQueue...
    // measure latency, verify correctness...
    std::cout << "Finished Demo\n";
*/
    const int numThreads = 4;
    const int opsPerThread = 100000;

    std::cout << "------ Wait-Free Algorithm Benchmarks Start ------\n";
    demo_wfstack(numThreads, opsPerThread);
    demo_wcq(numThreads, opsPerThread);
    demo_msqueue(numThreads, opsPerThread);
    std::cout << "------ Wait-Free Algorithm Benchmarks End ------\n";
    std::cout << "All benchmarks completed successfully.\n";
    return 0;
}
