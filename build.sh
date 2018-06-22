g++ -O3 -o shmq_send shmq_send.cc -lrt
g++ -O3 -o shmq_recv shmq_recv.cc -lrt

g++ -O3 -o multhread_q multhread_q.cc -lrt -pthread

g++ -O3 -o multhread_varq multhread_varq.cc -lrt -pthread

