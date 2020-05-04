#include "../SPSCQueue.h"
#include "../SPSCQueueOPT.h"
#include "shmmap.h"

struct Msg
{
    long ts;
    char buf[50];
};

typedef SPSCQueue<Msg, 4> MsgQueue;
// typedef SPSCQueueOPT<Msg, 4> MsgQueue;

MsgQueue* getMsgQueue() {
    return shmmap<MsgQueue>("/shm_queue");
}
