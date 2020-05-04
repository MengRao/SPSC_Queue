#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "../SPSCQueue.h"
#include "../SPSCQueueOPT.h"

struct Msg
{
  int val_len;
  long ts;
  long val[4];
};

// typedef SPSCQueue<Msg, 16> MsgQ;
typedef SPSCQueueOPT<Msg, 16> MsgQ;

const int loop = 100000;
const int sleep_cycles = 1000;
MsgQ _q;

void sendthread() {
  if (!cpupin(4)) {
    exit(1);
    }

    MsgQ* q = &_q;

    int g_val = 0;
    srand(time(NULL));
    while(g_val < loop) {
      q->tryPush([&](Msg* msg) {
        int val_len = rand() % 4 + 1;
        msg->val_len = val_len;
        for (int i = 0; i < val_len; i++) msg->val[i] = ++g_val;
        msg->ts = rdtscp();
      });
      auto expire = rdtsc() + sleep_cycles;
      while (rdtsc() < expire)
        ;
    }
}

void recvthread() {
  if (!cpupin(5)) {
    exit(1);
    }

    MsgQ* q = &_q;

    int cnt = 0;
    long sum_lat = 0;
    int g_val = 0;
    Msg* msg = nullptr;
    while(g_val < loop) {
      q->tryPop([&](Msg* msg) {
        long latency = rdtscp();
        latency -= msg->ts;
        sum_lat += latency;
        cnt++;
        for (int i = 0; i < msg->val_len; i++) assert(msg->val[i] == ++g_val);
      });
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


