#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "../SPSCQueue.h"
#include "../SPSCQueueOPT.h"

struct Msg
{
  int val_len;
  uint64_t ts;
  long val[4];
};

// typedef SPSCQueue<Msg, 64> MsgQ;
typedef SPSCQueueOPT<Msg, 64> MsgQ;

const int loop = 1000000;
const int sleep_cycles = 1000;
uint64_t alloc_lat = 0;
uint64_t push_lat = 0;
MsgQ _q;

void sendthread() {
  if (!cpupin(6)) {
    exit(1);
  }

    MsgQ* q = &_q;

    int g_val = 0;
    srand(time(NULL));
    while(g_val < loop) {
      auto t1 = rdtscp();
      Msg* msg = q->alloc();
      auto t2 = rdtscp();
      if (!msg) continue;
      int val_len = rand() % 4 + 1;
      msg->val_len = val_len;
      for (int i = 0; i < val_len; i++) msg->val[i] = ++g_val;
      auto t3 = rdtscp();
      msg->ts = t3;
      q->push();
      auto t4 = rdtscp();
      /*
      q->tryPush([&](Msg* msg) {
        int val_len = rand() % 4 + 1;
        msg->val_len = val_len;
        for (int i = 0; i < val_len; i++) msg->val[i] = ++g_val;
        msg->ts = rdtscp();
      });
      */
      alloc_lat += t2 - t1;
      push_lat += t4 - t3;
      auto expire = rdtsc() + sleep_cycles;
      while (rdtsc() < expire)
        ;
    }
}

void recvthread() {
  if (!cpupin(7)) {
    exit(1);
  }
    auto before = rdtscp();
    for (int i = 0; i < 99; i++) rdtscp();
    auto after = rdtscp();
    auto rdtscp_lat = (after - before) / 100;

    MsgQ* q = &_q;

    int cnt = 0;
    long sum_lat = 0;
    int g_val = 0;
    uint64_t front_lat = 0;
    uint64_t pop_lat = 0;
    while (g_val < loop) {
      auto t1 = rdtscp();
      Msg* msg = q->front();
      auto t2 = rdtscp();
      if (!msg) continue;
      sum_lat += t2 - msg->ts;
      cnt++;
      for (int i = 0; i < msg->val_len; i++) assert(msg->val[i] == ++g_val);
      auto t3 = rdtscp();
      q->pop();
      auto t4 = rdtscp();
      // q->pop2();
      front_lat += t2 - t1;
      pop_lat += t4 - t3;

      /*
      q->tryPop([&](Msg* msg) {
        long latency = rdtscp();
        latency -= msg->ts;
        sum_lat += latency;
        cnt++;
        for (int i = 0; i < msg->val_len; i++) assert(msg->val[i] == ++g_val);
      });
      */
    }
    std::cout << "recv done, val: " << g_val << " rdtscp_lat: " << rdtscp_lat << " avg_lat: " << (sum_lat / cnt)
              << " alloc_lat: " << (alloc_lat / cnt - rdtscp_lat) << " push_lat: " << (push_lat / cnt - rdtscp_lat)
              << " front_lat: " << (front_lat / cnt - rdtscp_lat) << " pop_lat: " << (pop_lat / cnt - rdtscp_lat)
              << std::endl;
}


int main() {
    std::thread trecv(recvthread);
    std::thread tsend(sendthread);

    tsend.join();
    trecv.join();

    return 0;
}


