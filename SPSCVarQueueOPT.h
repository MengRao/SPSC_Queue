#include <bits/stdc++.h>

/*
    SPSCVarQueueOPT is similar to SPSCVarQueue, and it provides an optimization that
    reading thread doesn't need to read write_idx to minimize memory footprint for reading thread,
    but writing thread must ensure the blk_sz of the end blk is 0(as a null blk similar to c string null char)
    Also note that the Block size here is 8 bytes instead of 64 bytes
    , in order to minimize the overhead of reading/writing the null blk
*/
template<uint32_t BLK_CNT>
class SPSCVarQueueOPT
{
public:
    static_assert(BLK_CNT && !(BLK_CNT & (BLK_CNT - 1)), "BLK_CNT must be a power of 2");

    struct Header
    {
        int16_t blk_sz; // < 0 means rewind
        uint16_t msg_type;
        // userdata can be used by caller, e.g. save timestamp or other stuff
        // we assume that user_msg is 8 types alligned so there'll be 4 bytes padding anyway, otherwise we can choose to
        // eliminate userdata
        uint32_t userdata;
    };

    template<class T>
    Header* alloc() {
        constexpr int16_t blk_sz = (sizeof(Header) + sizeof(T) + sizeof(Block) - 1) / sizeof(Block);
        static_assert(blk_sz + 1 <= BLK_CNT, "msg size is larger than queue size!");
        uint32_t padding_sz = BLK_CNT - (write_idx % BLK_CNT);
        bool rewind = blk_sz > padding_sz;
        uint32_t min_read_idx = write_idx + blk_sz + 1 + (rewind ? padding_sz : 0) - BLK_CNT; // +1 for the null block
        if((int)(read_idx_cach - min_read_idx) < 0) {
            asm volatile("" : "=m"(read_idx) : : ); // force read memory
            read_idx_cach = read_idx;
            if((int)(read_idx_cach - min_read_idx) < 0) { // no enough space
                return nullptr;
            }
        }
        if(rewind) {
            blk[0].header.blk_sz = 0;
            blk[write_idx % BLK_CNT].header.blk_sz = -(int32_t)padding_sz;
            write_idx += padding_sz;
        }
        blk[(write_idx + blk_sz) % BLK_CNT].header.blk_sz = 0;
        Header& header = blk[write_idx % BLK_CNT].header;
        header.msg_type = T::msg_type; // we can also let caller set msg_type since it's not needed by Queue
        return &header;
    }

    template<class T>
    void push() {
        constexpr int16_t blk_sz = (sizeof(Header) + sizeof(T) + sizeof(Block) - 1) / sizeof(Block);
        blk[write_idx % BLK_CNT].header.blk_sz = blk_sz;
        asm volatile("" : : "m"(blk) : ); // force write memory
        write_idx += blk_sz;
    }

    Header* front() {
        asm volatile("" : "=m"(blk) : : ); // force read memory
        int16_t blk_sz = blk[read_idx % BLK_CNT].header.blk_sz;
        if(blk_sz == 0) {
            return nullptr;
        }
        if(blk_sz < 0) {
            read_idx -= blk_sz;
            blk_sz = blk[0].header.blk_sz;
            if(blk_sz == 0) {
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
    struct Block // size of 8
    {
        Header header; // __attribute__((aligned(64)));
    } blk[BLK_CNT];

    // both write_idx and read_idx_cach are used by writing thread only
    alignas(64) uint32_t write_idx = 0;
    uint32_t read_idx_cach = 0;
    alignas(64) uint32_t read_idx = 0;
};


