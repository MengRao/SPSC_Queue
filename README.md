# SPSC_Queue
A single producer single consumer lock-free queue C++ template for ultimate low latency, which can be used in multithread conmunication as well as in shared memory IPC under Linux.

The latency of communication of a 10~200B message is within 50~100 ns, between two cpu cores on the same socket. And sender and receiver don't need to copy a single byte of the msg, that is, a msg is allocated in the queue memory and set by sender, and then read directly by the receiver in another thread/process.

This Library mainly contains two C++ templates, one for single typed queue and one for variant typed queue:

# SPSCQueue.h
A simple single typed queue template, with configurable msg type and queue size. 

# shmq_recv.cc/shmq_send.cc
A shared memory IPC example making use of the SPSCQueue.

# multhread_q.cc
A multi-thread example for testing the SPSCQueue.

# SPSCVarQueue.h
A general purpose variant typed queue template, with a header before each msg. The queue is partitioned in 64 bytes(typical cache line size) blocks and each msg(with header) is aligned with blocks so as to avoid writing a new msg false sharing with reading the old message.

# SPSCVarQueueOPT.h
An optimized version of SPSCVarQueue with the same interface. It differs in that the reading thread doesn't need to read write_idx so memory footprint is minimized, but the writing thread has to maintain additional null block invariant. Also the block size in this version is 8 bytes, in order to reduce the overhead of reading/writing the null block. Unfortunately, SPSCVarQueueOPT didn't show better performance over SPSCVarQueue under my tests, maybe it can be faster under your enviorment.

# multhread_varq.cc
A multi-thread example for testing SPSCVarQueue/SPSCVarQueueOPT

# build.sh
Building commands of above *.cc using g++, note c++11 is needed.
