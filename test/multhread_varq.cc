#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "../SPSCVarQueue.h"
#include "../SPSCVarQueueOPT.h"

// typedef SPSCVarQueue<1024 * 8> MsgQ;
typedef SPSCVarQueueOPT<1024 * 8> MsgQ;

using namespace std;

MsgQ _q;
const int loop = 1000000;
const int sleep_cycles = 1000;
uint64_t alloc_lat = 0;
uint64_t push_lat = 0;

void sendMsg(MsgQ* q, int n_long, int& g_val) {
  auto tsc = rdtscp();
  MsgQ::MsgHeader* header = q->alloc(n_long * sizeof(long));
  auto tsc2 = rdtscp();
  if (!header) return;
  alloc_lat += tsc2 - tsc;
  long* v = (long*)(header + 1);
  for (int i = 0; i < n_long; i++) v[i] = ++g_val;
  auto tsc3 = rdtscp();
  header->userdata = (uint32_t)tsc3;
  q->push();
  auto tsc4 = rdtscp();
  push_lat += tsc4 - tsc3;
}

void sendthread() {
  if (!cpupin(6)) {
    exit(1);
  }

    MsgQ* q = &_q;

    int g_val = 0;
    srand(time(NULL));
    while(g_val < loop) {
      int tp = rand() % 4 + 1;
      sendMsg(q, tp, g_val);
      auto expire = rdtsc() + sleep_cycles;
      while (rdtsc() < expire)
        ;
    }
}


void handleMsg(MsgQ::MsgHeader* header, int& g_val) {
  long* v = (long*)header + 1;
  long* end = (long*)((char*)header + header->size);
  while (v < end) assert(*v++ == ++g_val);
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
    uint64_t front_lat = 0;
    uint64_t pop_lat = 0;
    int g_val = 0;
    while(g_val < loop) {
      auto tsc = rdtscp();
      MsgQ::MsgHeader* header = q->front();
      auto tsc2 = rdtscp();
      if (header) {
        front_lat += tsc2 - tsc;
        long latency = (uint32_t)tsc2;
        latency -= header->userdata;
        if (latency < 0) latency += ((long)1 << 32);
        sum_lat += latency;
        cnt++;
        handleMsg(header, g_val);
        auto tsc3 = rdtscp();
        q->pop();
        auto tsc4 = rdtscp();
        pop_lat += tsc4 - tsc3;
      }
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



