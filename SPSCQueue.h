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
        T* ret = &data[read_idx % CNT];
        asm volatile("" : "=m"(data) : :); // memory fence
        return ret;
    }

    void pop() {
        asm volatile("" : "=m"(data) : "m"(read_idx) :); // memory fence
        ++read_idx;
        asm volatile("" : : "m"(read_idx) : ); // force write memory
    }

private:
    alignas(64) T data[CNT];
    alignas(64) uint32_t write_idx = 0;
    uint32_t read_idx_cach = 0; // used only by writing thread
    alignas(64) uint32_t read_idx = 0;
};

