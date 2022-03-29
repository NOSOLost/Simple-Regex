#pragma once
#ifndef PCC_ISTREAM_H_PCC_
#define PCC_ISTREAM_H_PCC_

#include <cstdio>
#include <fstream>

#include "pcc_config.h"
#include "pcc_template.h"

namespace pcc
{
template <typename Char_t>
class Std_stream;

template <>
class Std_stream<char> : public std::ifstream
{
    using std::ifstream::basic_ifstream;
};

template <>
class Std_stream<wchar_t> : public std::wifstream
{
    using std::wifstream::basic_ifstream;
};

template <typename Char_t>
class C_stream
{
    static_assert(is_same_v<Char_t, char> || is_same_v<Char_t, wchar_t>,
                  "C_stream require CharType to be char or wchar_t");

public:
    C_stream() : file(nullptr), count(0) {}

    C_stream(const Char_t* file_name) : file(fopen(file_name, "r")), count(0) {}

    void open(const Char_t* file_name) { file = fopen(file_name, "r"); }

    void close() { fclose(file); }

    bool is_open() { return file; }

    void read(Char_t* buffer, UInt size) { count = fread((void*)buffer, sizeof(Char_t), size, file); }

    size_t gcount() { return count; }

    bool eof() { return feof(file); }

private:
    FILE* file;
    size_t count;
};
}  // namespace pcc

#endif  // PCC_ISTREAM_H_PCC_