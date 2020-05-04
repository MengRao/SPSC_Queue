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

    template<typename Writer>
    bool tryPush(Writer writer) {
      T* p = alloc();
      if (!p) return false;
      writer(p);
      push();
      return true;
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
        bool avail = false;
        T data;
    } blk[CNT];
    char pad1[64];

    alignas(64) uint32_t write_idx = 0; // used only by writing thread
    char pad2[64];

    alignas(64) uint32_t read_idx = 0; // used only by reading thread
};


