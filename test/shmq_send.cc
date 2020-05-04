#include <bits/stdc++.h>
#include "rdtsc.h"
#include "cpupin.h"
#include "shmq_msg.h"

int main() {
    if(!cpupin(2)) {
        exit(1);
    }

    MsgQueue* q = getMsgQueue();

    Msg* msg = nullptr;
    while(1) {
        while((msg = q->alloc()) == nullptr)
            ;
        std::cout << "input: " << std::flush;
        if(!(std::cin >> msg->buf)) break;
        msg->ts = rdtsc();
        q->push();
    }

    return 0;
}

