# SPSC_Queue
A single producer single consumer lock-free queue C++ template for ultimate low latency, which can be used in multithread conmunication as well as in shared memory IPC under Linux.

The latency of communication of a 10-200B message is within 50-100 ns between two cpu cores on the same node. Sender and receiver don't need to copy a single byte of the msg, that is, a msg is allocated in the queue memory and set by sender, then read directly by the receiver in another thread/process.

This Library mainly contains two C++ templates, one for single typed queue and one for variant typed queue:

## SPSCQueue.h
A simple single typed queue template, with configurable msg type and queue size.

## SPSCQueueOPT.h
Another implementation of SPSCQueue, in which each msg has an additional bool header and write_idx and read_idx are not shared between threads. The performance is better or worse compared with SPSCQueue depending on the configuration. Note that Push() and Pop() operation are not atomic(check source code for details).

## shmq_recv.cc/shmq_send.cc
A shared memory IPC example for testing SPSCQueue/SPSCQueueOPT.

## multhread_q.cc
A multi-thread example for testing SPSCQueue/SPSCQueueOPT.

## SPSCVarQueue.h
A general purpose variant typed queue template, with a header before each msg. The queue is partitioned in 64 byte(typical cache line size) blocks and each msg(with header) is aligned with the block so as to avoid writing a new msg false sharing with reading the old message.

## multhread_varq.cc
A multi-thread example for testing SPSCVarQueue

## build.sh
Building commands of above *.cc using g++, note c++11 is needed.

# Best Practices
As in the examples, cpupin technique(SCHED_FIFO combined with cpu affinity) is used, which allows a thread to exclusively occupy a specified cpu core until it yields/exits. This is the recommended way in the realtime application where low and stable latency is required. But SCHED_FIFO may require root user permission.

Also note that we use rdtsc/rdtscp assembly instruction wrapper to fetch current timestamp in the test cases, in order to measure latency. rdtsc/rdtscp is the lowest overhead method to get timestamp on X86 host, it actually returns the cpu cycle count since the host booted, and the single counter is shared among all cpu cores so it can measure inter-core communication latency. To get the latency in time units, we can divide the cycle difference by CPU frenquency, e.g: 210 cycles on 3.1GHZ cpu is around 70ns. 

Also, as the msg memory is allocated on the queue and the allocation takes a bit of time, it's best to pre-allocate the msg you're about to send even when you don't have the entire msg content yet. However, this is only useful if you know the msg type beforehand.
