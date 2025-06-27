#include <gtest/gtest.h>
#include <wCQ_queue.h>
#include <thread>

TEST(WCQ, SingleThreadEnqueueDequeue) {
    WCQ<int> q(16, 1);
    int x = 42;
    EXPECT_TRUE(q.enqueue(new int(x), 0));
    auto out = q.dequeue(0);
    ASSERT_TRUE(out);
    EXPECT_EQ(**out, x);
    delete *out;
}

TEST(WCQ, MultiThreadStress) {
    const int N = 4, OPS = 10000;
    WCQ<int> q(128, N);
    auto worker = [&](int tid){
        for(int i = 0; i < OPS; ++i) {
            q.enqueue(new int(i), tid);
            auto out = q.dequeue(tid);
            if(out) delete *out;
        }
    };
    std::vector<std::thread> t;
    for(int i=0;i<N;i++) t.emplace_back(worker, i);
    for(auto &tt : t) tt.join();
    SUCCEED();
}
TEST(WCQ, EnqueueDequeueWithHelp) {
    const int N = 4, OPS = 1000;
    WCQ<int> q(16, N);
    auto worker = [&](int tid){
        for(int i = 0; i < OPS; ++i) {
            q.enqueue(new int(i + tid * OPS), tid);
            auto out = q.dequeue(tid);
            if(out) delete *out;
        }
    };
    std::vector<std::thread> t;
    for(int i=0;i<N;i++) t.emplace_back(worker, i);
    for(auto &tt : t) tt.join();
    SUCCEED();
}