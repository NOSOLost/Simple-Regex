#pragma once
#ifndef TEST_TOOLS_H_PCC_
#define TEST_TOOLS_H_PCC_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <vector>

#define TIME_OF(time)                                     \
    for (clock_t _Start = clock(), _Sign = 0; _Sign == 0; \
         (_Time) = (double)((double)clock() - (double)_Start) / CLOCKS_PER_SEC, _Sign = 1)

#define ADD_TIME_OF(time)                                 \
    for (clock_t _Start = clock(), _Sign = 0; _Sign == 0; \
         (_Time) += (double)((double)clock() - (double)_Start) / CLOCKS_PER_SEC, _Sign = 1)

#define assert_m(EXP, M)                               \
    do {                                               \
        if (!(EXP)) {                                  \
            std::cout << M << " ||||| " << std::flush; \
            assert(0);                                 \
        }                                              \
    } while (0)

namespace pcc_test
{
inline void check_assert(const char* const tru, const char* const fal)
{
    bool flag = false;
    assert(flag = true);
    std::cout << (flag ? tru : fal);
}

inline const char*& pstr()
{
    static const char* s;
    return s;
}

inline void static_message() { std::cout << pstr() << std::flush; }

inline void require_assert(const char* message)
{
    bool flag = false;
    assert(flag = true);
    if (!flag) {
        pstr() = message;
        atexit(static_message);
        exit(EXIT_FAILURE);
    }
}

inline std::ostream& print() { return std::cout; }

template <typename T, typename... Args>
inline std::ostream& print(T&& t, Args&&... args)
{
    std::cout << std::forward<T>(t);
    return print(std::forward<Args>(args)...);
}

template <typename... Args>
inline std::ostream& println(Args&&... args)
{
    return print(std::forward<Args>(args)...) << '\n';
}

template <typename Container, typename Outstream = std::ostream>
Outstream& show_container_assert_each(const Container& contr, const std::string& gap,
                                      std::function<bool(const typename Container::value_type&)> pred,
                                      Outstream& out = std::cout)
{
    out << "[ ";
    size_t size = contr.size();
    auto end = contr.end();
    int i = 0;
    for (auto now = contr.begin(); now != end; ++now, ++i) {
        out << *now;
        if (i != size - 1)
            out << gap;
        assert(pred(*now));
    }
    out << " ]";

    return out;
}

template <typename Container, typename Mapper, typename Outstream = std::ostream>
Outstream& show_container_map(const Container& contr, Mapper mapper, const std::string& gap = ", ", Outstream& out = std::cout)
{
    out << "[ ";
    size_t size = contr.size();
    auto end = contr.end();
    int i = 0;
    for (auto now = contr.begin(); now != end; ++now, ++i) {
        out << mapper(*now);
        if (i != size - 1)
            out << gap;
    }
    out << " ]";

    return out;
}

template <typename Container, typename Outstream = std::ostream>
Outstream& show_container(const Container& contr, const std::string& gap = ", ", Outstream& out = std::cout)
{
    return show_container_assert_each(
        contr, gap, [](const typename Container::value_type&) { return true; }, out);
}

template <typename Iter, typename Out = std::ostream>
Out& show_range(Iter beg, Iter end, const std::string gap = " ", Out& out = std::cout)
{
    for (Iter i = beg; i != end; ++i)
        out << *i << gap;
    return out;
}

template <typename T>
std::vector<T> get_rand_arr(size_t size, T beg, T last)
{
    static std::default_random_engine default_reng;

    std::vector<T> v(size);
    for (size_t i = 0; i < size; ++i)
        v[i] = std::uniform_int_distribution<T>(beg, last)(default_reng);

    return v;
}

inline std::vector<std::string> get_rand_str_arr(size_t size, size_t min_len, size_t max_len)
{
    static std::default_random_engine default_reng;

    std::vector<std::string> v(size);
    std::string str;
    for (size_t i = 0; i < size; ++i) {
        str = "";
        size_t str_size = std::uniform_int_distribution<size_t>(min_len, max_len)(default_reng);
        for (size_t c = 0; c < str_size; ++c)
            str += (char)std::uniform_int_distribution<int>('1', 'z')(default_reng);
        v.push_back(std::move(str));
    }

    return v;
}

template <typename T, typename Pred = std::less<T>>
bool is_sorted_with_std(const std::vector<T>& sorted, std::vector<T> ori, Pred pred = Pred())
{
    std::sort(ori.begin(), ori.end(), pred);
    return sorted == ori;
}
}  // namespace pcc_test

#endif  // TEST_TOOLS_H_NOSO_
