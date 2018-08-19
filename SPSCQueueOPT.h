
// Be aware that push()/pop() is not atomic.
// So if the queue is persisted in some shared memory and the writer/reader program crashed
// in the middle of push()/pop() call, the queue could be corrupted.
// Use SPSCQueue if this is a problem.
template<class T, uint32_t CNT>
class SPSCQueueOPT
{
public:

    T* alloc() {
        asm volatile("" : "=m"(blk) : :); // force read memory
        auto& cur_blk = blk[write_idx];
        if(cur_blk.avail) return nullptr; // no enough space
        return &cur_blk.data;
    }

    void push() {
        asm volatile("" : : "m"(blk) :); // memory fence
        auto& cur_blk = blk[write_idx];
        cur_blk.avail = true;
        asm volatile("" : : "m"(blk) :); // force write memory
        if(++write_idx == CNT) write_idx = 0;
    }

    T* front() {
        asm volatile("" : "=m"(blk) : :); // force read memory
        auto& cur_blk = blk[read_idx];
        if(!cur_blk.avail) return nullptr;
        asm volatile("" : "=m"(blk) : :); // memory fence
        return &cur_blk.data;
    }

    void pop() {
        asm volatile("" : "=m"(blk) : "m"(blk) :); // memory fence
        auto& cur_blk = blk[read_idx];
        cur_blk.avail = false;
        asm volatile("" : : "m"(blk) :); // force write memory
        if(++read_idx == CNT) read_idx = 0;
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


