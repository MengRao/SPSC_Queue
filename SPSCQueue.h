#include <bits/stdc++.h>

template<class T, uint32_t CNT>
class SPSCQueue
{
public:
    static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

    T* alloc() {
        if(write_idx - read_idx_cach == CNT) {
            asm volatile("" : "=m"(read_idx) : : ); // force read memory
            read_idx_cach = read_idx; 
            if(__builtin_expect(write_idx - read_idx_cach == CNT, 0)) { // no enough space
                return nullptr;
            }
        }
        return &data[write_idx % CNT];
    }

    void push() {
        asm volatile("" : : "m"(data), "m"(write_idx) :); // memory fence
        ++write_idx;
        asm volatile("" : : "m"(write_idx) : ); // force write memory
    }

    T* front() {
        asm volatile("" : "=m"(write_idx) : : ); // force read memory
        if(read_idx == write_idx) {
            return nullptr;
        }
        asm volatile("" : "=m"(write_idx), "=m"(data) : :); // memory fence
        return &data[read_idx % CNT];
    }

    void pop() {
        ++read_idx;
        asm volatile("" : : "m"(read_idx) : ); // force write memory
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

