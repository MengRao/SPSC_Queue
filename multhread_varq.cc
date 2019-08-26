#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "SPSCVarQueue.h"

typedef SPSCVarQueue<1024> MsgQ;

template<uint32_t N, uint16_t MSGTYPE>
struct Msg
{
    static constexpr uint16_t msg_type = MSGTYPE;
    long val[N];
};

typedef Msg<1, 1> Msg1;
typedef Msg<2, 2> Msg2;
typedef Msg<3, 3> Msg3;
typedef Msg<4, 4> Msg4;

MsgQ _q;
const int loop = 100000;
const int sleep_cycles = 1000;

template<class T>
void sendMsg(MsgQ* q, int& g_val) {
  q->tryPush(sizeof(T), [&](MsgQ::MsgHeader* header) {
    header->msg_type = T::msg_type;
    T* msg = (T*)(header + 1);
    for (auto& v : msg->val) v = ++g_val;
    header->userdata = (uint32_t)rdtscp();
  });
}

void sendthread() {
  if (!cpupin(4)) {
    exit(1);
    }

    MsgQ* q = &_q;

    int g_val = 0;
    srand(time(NULL));
    while(g_val < loop) {
      // std::this_thread::yield();
      int tp = rand() % 4 + 1;
      switch (tp) {
        case 1: sendMsg<Msg1>(q, g_val); break;
        case 2: sendMsg<Msg2>(q, g_val); break;
        case 3: sendMsg<Msg3>(q, g_val); break;
        case 4: sendMsg<Msg4>(q, g_val); break;
        }
        auto expire = rdtsc() + sleep_cycles;
        while (rdtsc() < expire)
          ;
    }
}

template<class T>
void handleMsg(MsgQ::MsgHeader* header, int& g_val) {
    T* msg = (T*)(header + 1);
    for(auto v : msg->val) assert(v == ++g_val);
    // for(int i = 0; i < sizeof(msg->val) / sizeof(*msg->val); i++) assert(msg->val[i] == ++g_val);
}

void recvthread() {
  if (!cpupin(5)) {
    exit(1);
    }

    MsgQ* q = &_q;

    int cnt = 0;
    long sum_lat = 0;
    int g_val = 0;
    while(g_val < loop) {
      q->tryPop([&](MsgQ::MsgHeader* header) {
        long latency = (uint32_t)rdtscp();
        latency -= header->userdata;
        if (latency < 0) latency += ((long)1 << 32);
        sum_lat += latency;
        cnt++;
        switch (header->msg_type) {
          case 1: handleMsg<Msg1>(header, g_val); break;
          case 2: handleMsg<Msg2>(header, g_val); break;
          case 3: handleMsg<Msg3>(header, g_val); break;
          case 4: handleMsg<Msg4>(header, g_val); break;
        }
      });
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



