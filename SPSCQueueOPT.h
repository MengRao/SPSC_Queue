#include <bits/stdc++.h>

template<class T, uint32_t CNT>
class SPSCQueueOPT
{
public:
    static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

    T* alloc() {
        auto& cur_blk = blk[write_idx % CNT];
        asm volatile("" : "=m"(cur_blk.avail) : :); // force read memory
        if(cur_blk.avail) return nullptr;           // no enough space
        return &cur_blk.data;
    }

    void push() {
        auto& cur_blk = blk[write_idx % CNT];
        cur_blk.avail = true;
        asm volatile("" : : "m"(cur_blk) :); // force write memory
        ++write_idx;
    }

    T* front() {
        auto& cur_blk = blk[read_idx % CNT];
        asm volatile("" : "=m"(cur_blk) : :); // force read memory
        if(!cur_blk.avail) return nullptr;
        return &cur_blk.data;
    }

    void pop() {
        auto& cur_blk = blk[read_idx % CNT];
        cur_blk.avail = false;
        asm volatile("" : : "m"(cur_blk.avail) :); // force write memory
        ++read_idx;
    }

    void print() {
        std::cout << "write_idx: " << write_idx << " read_idx: " << read_idx << std::endl;
    }

private:
    struct alignas(64) Block
    {
        bool avail = false;
        T data;
    } blk[CNT];
    alignas(64) uint32_t write_idx = 0; // used only by writing thread
    alignas(64) uint32_t read_idx = 0;  // used only by reading thread
};


