# SPSC_Queue
A single producer single consumer lock-free queue C++ template for ultimate low latency, which can be used in multithread conmunication as well as in shared memory IPC under Linux.

The latency of communication of a 10-200B message is within 50-100 ns between two cpu cores on the same node. Sender and receiver don't need to copy a single byte of the msg, that is, a msg is allocated in the queue memory and set by sender, then read directly by the receiver in another thread/process.

This Library mainly contains two C++ templates, one for single typed queue and one for variant typed queue:

## SPSCQueue.h
An atomic(crash safe when used in shared-memory IPC) single typed queue template.

## SPSCQueueOPT.h
An optimized implementation of SPSCQueue, in which each msg has an additional bool header and write_idx and read_idx are not shared between threads. Note that Push() and Pop() operation are not atomic so it should only be used in multithreaded programming.

## shmq_recv.cc/shmq_send.cc
A shared memory IPC example for testing SPSCQueue/SPSCQueueOPT.

## multhread_q.cc
A multi-thread example for testing SPSCQueue/SPSCQueueOPT.

## SPSCVarQueue.h
An atomic(crash safe when used in shared-memory IPC) general purpose variant typed queue template, with a header before each msg.

## SPSCVarQueueOPT.h
An optimized implementation of SPSCVarQueue, in which reader doens't need to read write_idx so latency is reduced when new msg comes. Note that SPSCVarQueueOPT is not atomic.

## multhread_varq.cc
A multi-thread example for testing SPSCVarQueue

## build.sh
Building commands of above *.cc using g++, note c++11 is needed.

