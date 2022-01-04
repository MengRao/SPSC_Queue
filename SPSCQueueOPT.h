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

// Be aware that push()/pop() is not atomic.
// So if the queue is persisted in some shared memory and the writer/reader program crashed
// in the middle of push()/pop() call, the queue could be corrupted.
// Use SPSCQueue if this is a problem.
template<class T, uint32_t CNT>
class SPSCQueueOPT
{
public:
  static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

  T* alloc() {
    if (free_write_cnt == 0) {
      uint32_t rd_idx = ((std::atomic<uint32_t>*)&read_idx)->load(std::memory_order_consume);
      free_write_cnt = (rd_idx - write_idx + CNT - 1) % CNT;
      if (__builtin_expect(free_write_cnt == 0, 0)) return nullptr;
    }
    return &blk[write_idx].data;
  }

  void push() {
    ((std::atomic<bool>*)&blk[write_idx].avail)->store(true, std::memory_order_release);
    write_idx = (write_idx + 1) % CNT;
    free_write_cnt--;
  }

  template<typename Writer>
  bool tryPush(Writer writer) {
    T* p = alloc();
    if (!p) return false;
    writer(p);
    push();
    return true;
  }

  template<typename Writer>
  void blockPush(Writer writer) {
    while (!tryPush(writer))
      ;
  }

  T* front() {
    auto& cur_blk = blk[read_idx];
    if (!((std::atomic<bool>*)&cur_blk.avail)->load(std::memory_order_acquire)) return nullptr;
    return &cur_blk.data;
  }

  void pop() {
    blk[read_idx].avail = false;
    ((std::atomic<uint32_t>*)&read_idx)->store((read_idx + 1) % CNT, std::memory_order_release);
  }

  template<typename Reader>
  bool tryPop(Reader reader) {
    T* v = front();
    if (!v) return false;
    reader(v);
    pop();
    return true;
  }

private:
  struct alignas(64) Block
  {
    bool avail = false; // avail will be updated by both write and read thread
    T data;
  } blk[CNT] = {};

  alignas(128) uint32_t write_idx = 0; // used only by writing thread
  uint32_t free_write_cnt = CNT - 1;

  alignas(128) uint32_t read_idx = 0;
};


