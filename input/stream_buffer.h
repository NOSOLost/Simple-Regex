#pragma once
#ifndef STREAM_BUFFER_H_PCC_
#define STREAM_BUFFER_H_PCC_

#include <cstdio>

#include "buffer.h"
#include "pcc_config.h"

namespace pcc
{
template <typename Char_t, UInt SIZE, typename Stream>
class Stream_buff
{
public:
    Stream_buff() : stream(nullptr), buff() {}

    Stream_buff(Char_t* beg, Char_t* end) : stream(nullptr), buff(beg, end) {}

    template <typename NStream>
    Stream_buff(Char_t* beg, Char_t* end, NStream& s) : Stream_buff(beg, end)
    {
        reset_stream(s);
    }

    template <typename NStream>
    void reset_stream(NStream& s)
    {
        stream = &s;
    }

    void reset_buff_memory(Char_t* beg, Char_t* end) { buff.reset_memory(beg, end); }

    void fill_buff()
    {
        buff.dec();
        fill_char();
        buff.inc();
        fill_buff_aux();
        buff.dec();
    }

    bool has_stream() { return !stream->eof(); }

    void inc() { buff.inc(); }

    void roll_back(UInt sz) { buff.roll_back(sz); }

    void set_cur_char(Char_t c) { buff.set_cur_elem(c); }

    Char_t next()
    {
        Char_t c = buff.cur_elem();
        buff.inc();
        return c;
    }

private:
    void fill_char() { stream->read(buff.begin() + buff.cursor(), 1); }

    void fill_buff_aux()
    {
        assert(buff.cursor() == 0 || buff.cursor() == buff.BUFF_SIZE);
        Char_t* beg = buff.begin() + buff.cursor();
        stream->read(beg, buff.BUFF_SIZE - 1);
        *(beg + stream->gcount()) = EOF;
    }

    Stream* stream;
    Buffer<Char_t, SIZE> buff;
};
}  // namespace pcc

#endif  // STREAM_BUFFER_H_PCC_