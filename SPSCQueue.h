#include <bits/stdc++.h>

template<class T, uint32_t CNT>
class SPSCQueue
{
public:
    static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

    T* alloc() {
        if(write_idx - read_idx_cach == CNT) {
            read_idx_cach = *(volatile uint32_t*)&read_idx; // force read memory
            if(__builtin_expect(write_idx - read_idx_cach == CNT, 0)) { // no enough space
                return nullptr;
            }
        }
        return &data[write_idx % CNT];
    }

    void push() {
        std::atomic_thread_fence(std::memory_order_relaxed); // force write memory
        ++write_idx;
    }

    T* front() {
        if(read_idx == *(volatile uint32_t*)&write_idx) {
            return nullptr;
        }
        return &data[read_idx % CNT];
    }

    void pop() {
        std::atomic_thread_fence(std::memory_order_relaxed); // force write memory
        ++read_idx;
    }

    void print() {
        std::cout << "write_idx: " << write_idx << " read_idx: " << read_idx << std::endl;
    }

private:
    T data[CNT] __attribute__((aligned(64)));
    uint32_t write_idx __attribute__((aligned(64))) = 0;
    uint32_t read_idx_cach = 0; // used only by writing thread
    uint32_t read_idx __attribute__((aligned(64))) = 0;
};

