#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "SPSCVarQueue.h"
#include "SPSCVarQueueOPT.h"

typedef SPSCVarQueue<32> MsgQ; // blk size is 64B
// typedef SPSCVarQueueOPT<256> MsgQ; // blk size is 8B

template<uint32_t N, uint16_t MSGTYPE>
struct Msg
{
    static constexpr uint16_t msg_type = MSGTYPE;
    long val[N];
};

typedef Msg<3, 1> Msg1;
typedef Msg<8, 2> Msg2;
typedef Msg<17, 3> Msg3;
typedef Msg<45, 4> Msg4;

MsgQ _q;
const int loop = 100000;

template<class T>
void sendMsg(MsgQ* q, int& g_val) {
    MsgQ::Header* header = nullptr;
    while((header = q->alloc<T>()) == nullptr)
        ;
    T* msg = (T*)(header + 1);
    for(auto& v : msg->val) v = ++g_val;
    // for(int i = 0; i < sizeof(msg->val) / sizeof(*msg->val); i++) msg->val[i] = ++g_val;
    header->userdata = (uint32_t)rdtscp();
    q->push<T>();
}

void sendthread() {
    if(!cpupin(2)) {
        exit(1);
    }

    MsgQ* q = &_q;

    int g_val = 0;
    srand(time(NULL));
    while(g_val < loop) {
        std::this_thread::yield();
        int tp = rand() % 4 + 1;
        if(tp == 1) {
            sendMsg<Msg1>(q, g_val);
        }
        else if(tp == 2) {
            sendMsg<Msg2>(q, g_val);
        }
        else if(tp == 3) {
            sendMsg<Msg3>(q, g_val);
        }
        else if(tp == 4) {
            sendMsg<Msg4>(q, g_val);
        }
        // std::cout << "send tp: " << tp << " g_val: " << g_val << std::endl;
    }
    // std::cout << "shmq_send done, val: " << g_val << std::endl;
}

template<class T>
void handleMsg(MsgQ::Header* header, int& g_val) {
    T* msg = (T*)(header + 1);
    for(auto v : msg->val) assert(v == ++g_val);
    // for(int i = 0; i < sizeof(msg->val) / sizeof(*msg->val); i++) assert(msg->val[i] == ++g_val);
}

void recvthread() {
    if(!cpupin(3)) {
        exit(1);
    }

    MsgQ* q = &_q;

    int cnt = 0;
    long sum_lat = 0;
    int g_val = 0;
    while(g_val < loop) {
        MsgQ::Header* header = q->front();
        if(header == nullptr) continue;
        long latency = (uint32_t)rdtscp();
        latency -= header->userdata;
        if(latency < 0) latency += ((long)1 << 32);
        sum_lat += latency;
        cnt++;
        auto msg_type = header->msg_type;
        if(msg_type == 1) {
            handleMsg<Msg1>(header, g_val);
        }
        else if(msg_type == 2) {
            handleMsg<Msg2>(header, g_val);
        }
        else if(msg_type == 3) {
            handleMsg<Msg3>(header, g_val);
        }
        else if(msg_type == 4) {
            handleMsg<Msg4>(header, g_val);
        }
        else {
            std::cout << "error, unknown msg_type: " << msg_type << std::endl;
        }
        // std::cout << "recv tp: " << msg_type << " g_val: " << g_val << " latency: " << latency << std::endl;
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



