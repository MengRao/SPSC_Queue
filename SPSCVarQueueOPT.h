/*
MIT License

Copyright (c) 2018 Meng Rao <raomeng1@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <atomic>

// alloc and push are not atomic, so SPSCVarQueueOPT should not be used in shared-memory IPC(use SPSCVarQueue instead)
template<uint32_t Bytes>
class SPSCVarQueueOPT
{
public:
  struct MsgHeader
  {
    // size of this msg, including header itself
    // auto set by lib, can be read by user
    uint16_t size;
    uint16_t msg_type;
    // userdata can be used by caller, e.g. save timestamp or other stuff
    // we assume that user_msg is 8 types alligned so there'll be 4 bytes padding anyway, otherwise we can choose to
    // eliminate userdata
    uint32_t userdata;
  };
  static constexpr uint32_t BLK_CNT = Bytes / sizeof(MsgHeader);

  MsgHeader* alloc(uint16_t size_) {
    size = size_ + sizeof(MsgHeader);
    uint32_t blk_sz = (size + sizeof(MsgHeader) - 1) / sizeof(MsgHeader);
    if (blk_sz >= free_write_cnt) {
      uint32_t read_idx_cache = *(volatile uint32_t*)&read_idx;
      if (read_idx_cache <= write_idx) {
        free_write_cnt = BLK_CNT - write_idx;
        if (blk_sz >= free_write_cnt && read_idx_cache != 0) { // wrap around
          blk[0].size = 0;
          std::atomic_thread_fence(std::memory_order_release);
          blk[write_idx].size = 1;
          write_idx = 0;
          free_write_cnt = read_idx_cache;
        }
      }
      else {
        free_write_cnt = read_idx_cache - write_idx;
      }
      if (free_write_cnt <= blk_sz) {
        return nullptr;
      }
    }
    return &blk[write_idx];
  }

  void push() {
    uint32_t blk_sz = (size + sizeof(MsgHeader) - 1) / sizeof(MsgHeader);
    blk[write_idx + blk_sz].size = 0;
    std::atomic_thread_fence(std::memory_order_release);

    blk[write_idx].size = size;
    write_idx += blk_sz;
    free_write_cnt -= blk_sz;
  }

  template<typename Writer>
  bool tryPush(uint16_t size, Writer writer) {
    MsgHeader* header = alloc(size);
    if (!header) return false;
    writer(header);
    push();
    return true;
  }

  template<typename Writer>
  void blockPush(uint16_t size, Writer writer) {
    while (!tryPush(size, writer))
      ;
  }

  MsgHeader* front() {
    uint16_t size = blk[read_idx].size;
    if (size == 1) { // wrap around
      read_idx = 0;
      size = blk[0].size;
    }
    if (size == 0) return nullptr;
    return &blk[read_idx];
  }

  void pop() {
    uint32_t blk_sz = (blk[read_idx].size + sizeof(MsgHeader) - 1) / sizeof(MsgHeader);
    *(volatile uint32_t*)&read_idx = read_idx + blk_sz;
  }

  template<typename Reader>
  bool tryPop(Reader reader) {
    MsgHeader* header = front();
    if (!header) return false;
    reader(header);
    pop();
    return true;
  }

private:
  alignas(64) MsgHeader blk[BLK_CNT] = {};

  alignas(128) uint32_t write_idx = 0;
  uint32_t free_write_cnt = BLK_CNT;
  uint16_t size;

  alignas(128) uint32_t read_idx = 0;
};

