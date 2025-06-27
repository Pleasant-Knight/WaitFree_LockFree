#include <wCQ_queue.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main(){
    const int N = 4, OPS = 100000;
    WCQ<int> q(1024, N);
    auto work = [&](int tid){
        for(int i=0;i<OPS;++i){
            q.enqueue(new int(i), tid);
            auto out = q.dequeue(tid);
            if(out) delete *out;
        }
    };
    auto start = std::chrono::steady_clock::now();
    std::vector<std::thread> t;
    for(int i=0;i<N;i++) t.emplace_back(work, i);
    for(auto &tt : t) tt.join();
    double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << "Throughput: " << (N * OPS * 2) / elapsed << " ops/sec\n";
    return 0;
}
