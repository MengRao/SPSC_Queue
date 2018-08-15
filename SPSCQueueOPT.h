
template<class T, uint32_t CNT>
class SPSCQueueOPT
{
public:

    T* alloc() {
        auto& cur_blk = blk[write_idx];
        asm volatile("" : "=m"(cur_blk.avail) : :); // force read memory
        if(cur_blk.avail) return nullptr;           // no enough space
        return &cur_blk.data;
    }

    void push() {
        auto& cur_blk = blk[write_idx];
        cur_blk.avail = true;
        asm volatile("" : : "m"(cur_blk) :); // force write memory
        if(++write_idx == CNT) write_idx = 0;
    }

    T* front() {
        auto& cur_blk = blk[read_idx];
        asm volatile("" : "=m"(cur_blk) : :); // force read memory
        if(!cur_blk.avail) return nullptr;
        return &cur_blk.data;
    }

    void pop() {
        auto& cur_blk = blk[read_idx];
        cur_blk.avail = false;
        asm volatile("" : : "m"(cur_blk.avail) :); // force write memory
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


