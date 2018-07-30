g++ -std=c++11 -O3 -o shmq_send shmq_send.cc -lrt
g++ -std=c++11 -O3 -o shmq_recv shmq_recv.cc -lrt

g++ -std=c++11 -O3 -o multhread_q multhread_q.cc -lrt -pthread

g++ -std=c++11 -O3 -o multhread_varq multhread_varq.cc -lrt -pthread

