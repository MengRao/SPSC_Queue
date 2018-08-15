#include <bits/stdc++.h>

template<uint32_t BLK_CNT>
class SPSCVarQueue
{
public:
    static_assert(BLK_CNT && !(BLK_CNT & (BLK_CNT - 1)), "BLK_CNT must be a power of 2");

    struct Header
    {
        int16_t blk_sz; // < 0 means rewind
        uint16_t msg_type;
        // userdata can be used by caller, e.g. saving timestamp or other stuff
        // we assume that user_msg is 8 types alligned so there'll be 4 bytes padding anyway, otherwise we can choose to
        // eliminate userdata
        uint32_t userdata;
    };

    template<class T>
    Header* alloc() {
        constexpr int16_t blk_sz = (sizeof(Header) + sizeof(T) + sizeof(Block) - 1) / sizeof(Block);
        static_assert(blk_sz <= BLK_CNT, "msg size is larger than queue size!");
        uint32_t padding_sz = BLK_CNT - (write_idx % BLK_CNT);
        bool rewind = blk_sz > padding_sz;
        // min_read_idx could negtive which results in a large unsigned int
        uint32_t min_read_idx = write_idx + blk_sz + (rewind ? padding_sz : 0) - BLK_CNT;
        if((int)(read_idx_cach - min_read_idx) < 0) {
            asm volatile("" : "=m"(read_idx) : : ); // force read memory
            read_idx_cach = read_idx;
            if((int)(read_idx_cach - min_read_idx) < 0) { // no enough space
                return nullptr;
            }
        }
        if(rewind) {
            blk[write_idx % BLK_CNT].header.blk_sz = -(int32_t)padding_sz;
            write_idx += padding_sz;
        }
        Header& header = blk[write_idx % BLK_CNT].header;
        header.blk_sz = blk_sz;
        header.msg_type = T::msg_type; // we can also let caller set msg_type since it's not needed by Queue
        return &header;
    }

    template<class T>
    void push() {
        constexpr int16_t blk_sz = (sizeof(Header) + sizeof(T) + sizeof(Block) - 1) / sizeof(Block);
        write_idx += blk_sz;
        asm volatile("" : : "m"(write_idx) : ); // force write memory
    }

    Header* front() {
        asm volatile("" : "=m"(write_idx) : : ); // force read memory
        if(read_idx == write_idx) {
            return nullptr;
        }
        auto blk_sz = blk[read_idx % BLK_CNT].header.blk_sz;
        if(blk_sz < 0) { // rewind
            read_idx -= blk_sz;
            if(read_idx == write_idx) {
                return nullptr;
            }
        }
        return &blk[read_idx % BLK_CNT].header;
    }

    void pop() {
        read_idx += blk[read_idx % BLK_CNT].header.blk_sz;
        asm volatile("" : : "m"(read_idx) : ); // force write memory
    }

    void print() {
        std::cout << "write_idx: " << write_idx << " read_idx: " << read_idx << std::endl;
    }

private:
    struct alignas(64) Block // size of 64, same as cache line
    {
        Header header;
    } blk[BLK_CNT];

    alignas(64) uint32_t write_idx = 0;
    uint32_t read_idx_cach = 0; // used only by writing thread
    alignas(64) uint32_t read_idx = 0;
};

