#include <bits/stdc++.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

template<class T>
T* shmmap(const char * filename) {
    int shm_fd = shm_open(filename, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1) {
        std::cerr << "shm_open failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    if(ftruncate(shm_fd, sizeof(T))) {
        std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
        close(shm_fd);
        return nullptr;
    }
    T* ret = (T*)mmap(0, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if(ret == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    return ret;
}
