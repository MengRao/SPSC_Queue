#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "shmq_msg.h"

int main() {
    if(!cpupin(3)) {
        exit(1);
    }

    MsgQueue* q = getMsgQueue();

    Msg* msg = nullptr;
    while(1) {
        while((msg = q->front()) == nullptr)
            ;
        long latency = rdtsc();
        latency -= msg->ts;
        std::cout << "recv: " << msg->buf << " ,latency: " << latency << std::endl;
        q->pop();
    }

    return 0;
}


