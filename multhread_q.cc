#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "SPSCQueue.h"
#include "SPSCQueueOPT.h"

struct Msg
{
    long ts;
    long val[5];
};

typedef SPSCQueue<Msg, 8> MsgQ;
// typedef SPSCQueueOPT<Msg, 8> MsgQ;

const int loop = 10000000;
MsgQ _q;

void sendthread() {
    if(!cpupin(2)) {
        exit(1);
    }

    MsgQ* q = &_q;

    int g_val = 0;
    Msg* msg = nullptr;
    while(g_val < loop) {
        std::this_thread::yield();
        while((msg = q->alloc()) == nullptr)
            ;
        // for(int i = 0; i < sizeof(msg->val) / sizeof(msg->val[0]); i++) msg->val[i] = ++g_val;
        for(auto& v : msg->val) v = ++g_val;
        msg->ts = rdtsc();
        q->push();
        // std::cout << "send g_val: " << g_val << std::endl;
    }
    // std::cout << "shmq_send done, val: " << g_val << std::endl;
}

void recvthread() {
    if(!cpupin(3)) {
        exit(1);
    }

    MsgQ* q = &_q;
    q->print();

    int cnt = 0;
    long sum_lat = 0;
    int g_val = 0;
    Msg* msg = nullptr;
    while(g_val < loop) {
        while((msg = q->front()) == nullptr)
            ;
        long latency = rdtsc();
        latency -= msg->ts;
        sum_lat += latency;
        cnt++;
        for(auto v : msg->val) assert(v == ++g_val);
        q->pop();
    }

    std::cout << "shmq_recv done, val: " << g_val << " avg_lat: " << (sum_lat / cnt) << std::endl;
}


int main() {
    std::thread trecv(recvthread);
    std::thread tsend(sendthread);

    tsend.join();
    trecv.join();

    return 0;
}




