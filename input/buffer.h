#pragma once
#ifndef BUFFER_H_PCC_
#define BUFFER_H_PCC_

#include "pcc_config.h"
#include "pcc_template.h"

namespace pcc
{
template <typename T, UInt SIZE>
class Buffer
{
public:
    Buffer() : Buffer(nullptr, nullptr) {}

    Buffer(T* b, T* e) : begin_(b), end_(e), cur(0) { assert(e - b == SIZE); }

    void reset_memory(T* b, T* e)
    {
        assert(b - e == BUFF_SIZE);
        begin_ = b, end_ = e;
    }

    T* begin() { return begin_; }

    T* end() { return end_; }

    UInt cursor() { return cur; }

    void add(UInt n) { cur = (cur + n) & MAX_INDEX; }

    void roll_back(UInt n) { cur = (cur - n) & MAX_INDEX; }

    void inc() { add(1); }

    void dec() { roll_back(1); }

    T cur_elem() { return *(begin() + cur); }

    T next_elem()
    {
        inc();
        return cur_elem();
    }

    void set_cur_elem(T t) { *(begin() + cur) = t; }

    static_assert(SIZE > 2 && is_2_n<SIZE>::type::val, "The size of Buffer is required to be 1 << n (n > 1)");
    static constexpr UInt MAX_INDEX = SIZE - 1;
    static constexpr UInt BUFF_SIZE = SIZE >> 1;

private:
    T* begin_;
    T* end_;
    UInt cur;
};
}  // namespace pcc

#endif  // BUFFER_H_PCC_