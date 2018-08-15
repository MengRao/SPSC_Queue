
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

private:
    alignas(64) T data[CNT];
    alignas(64) uint32_t write_idx = 0;
    uint32_t read_idx_cach = 0; // used only by writing thread
    alignas(64) uint32_t read_idx = 0;
};

