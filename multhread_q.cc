#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "SPSCQueue.h"
#include "SPSCQueueOPT.h"

struct Msg
{
    long ts;
    long val[1];
};

typedef SPSCQueue<Msg, 2> MsgQ;
// typedef SPSCQueueOPT<Msg, 2> MsgQ;

const int loop = 10000000;
MsgQ _q;

void sendthread() {
  if (!cpupin(2)) {
    exit(1);
    }

    MsgQ* q = &_q;

    int g_val = 0;
    while(g_val < loop) {
      while (!q->tryPush([&](Msg* msg) {
        for (auto& v : msg->val) v = ++g_val;
        msg->ts = rdtsc();
      }))
        ;
    }
}

void recvthread() {
  if (!cpupin(3)) {
    exit(1);
    }

    MsgQ* q = &_q;

    int cnt = 0;
    long sum_lat = 0;
    int g_val = 0;
    Msg* msg = nullptr;
    while(g_val < loop) {
      while (!q->tryPop([&](Msg* msg) {
        long latency = rdtsc();
        latency -= msg->ts;
        sum_lat += latency;
        cnt++;
        for(auto v : msg->val) assert(v == ++g_val);
      }))
        ;
    }

    std::cout << "recvthread done, val: " << g_val << " avg_lat: " << (sum_lat / cnt) << std::endl;
}


int main() {
    std::thread trecv(recvthread);
    std::thread tsend(sendthread);

    tsend.join();
    trecv.join();

    return 0;
}




