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

template<class T, uint32_t CNT>
class SPSCQueue
{
public:
    static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");

    T* alloc() {
      if (write_idx - read_idx_cach == CNT) {
        read_idx_cach = ((std::atomic<uint32_t>*)&read_idx)->load(std::memory_order_consume);
        if (__builtin_expect(write_idx - read_idx_cach == CNT, 0)) { // no enough space
          return nullptr;
        }
      }
      return &data[write_idx % CNT];
    }

    void push() {
      ((std::atomic<uint32_t>*)&write_idx)->store(write_idx + 1, std::memory_order_release);
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
        if (read_idx == ((std::atomic<uint32_t>*)&write_idx)->load(std::memory_order_acquire)) {
          return nullptr;
        }
        return &data[read_idx % CNT];
    }

    void pop() {
      ((std::atomic<uint32_t>*)&read_idx)->store(read_idx + 1, std::memory_order_release);
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
    alignas(128) T data[CNT] = {};

    alignas(128) uint32_t write_idx = 0;
    uint32_t read_idx_cach = 0; // used only by writing thread

    alignas(128) uint32_t read_idx = 0;
};

