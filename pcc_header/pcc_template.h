#pragma once
#ifndef PCC_TEMPLATE_H_PCC_
#define PCC_TEMPLATE_H_PCC_

#include <cstdint>

#include "pcc_config.h"
#ifdef HAS_BOOST
#include "boost/container/small_vector.hpp"
#endif

namespace pcc
{
template <typename T>
using Vector = std::vector<T>;

#ifdef HAS_BOOST
template <typename T, size_t SIZE>
using Small_vector = boost::container::small_vector<T, SIZE>;
#else
template <typename T, size_t SIZE>
using Small_vector = Vector<T>;
#endif

template <typename Key, typename Value>
using Hash_map = std::unordered_map<Key, Value>;

template <typename Key, typename Value>
using Mul_hash_map = std::unordered_multimap<Key, Value>;

template <typename Key>
using Hash_set = std::unordered_set<Key>;

template <typename Key>
using Mul_hash_set = std::unordered_multiset<Key>;

template <bool value>
struct Boolean {
    static constexpr bool val = value;
};
using True = Boolean<true>;
using False = Boolean<false>;

template <typename T1, typename T2>
struct is_same_s {
    using type = False;
};
template <typename T>
struct is_same_s<T, T> {
    using type = True;
};
template <typename T1, typename T2>
using is_same = typename is_same_s<T1, T2>::type;

template <typename T1, typename T2>
inline constexpr bool is_same_v = is_same_s<T1, T2>::type::val;

template <size_t ui>
struct is_2_n {
    static constexpr bool val = (ui == 02 ? true : ((ui & 01) == 0 ? is_2_n<(ui >> 1)>::val : false));
    using type = Boolean<val>;
};
template <>
struct is_2_n<1> {
    static constexpr bool val = false;
    using type = Boolean<val>;
};
template <>
struct is_2_n<0> {
    static constexpr bool val = true;
    using type = Boolean<val>;
};

template <size_t MULTIPLE>
inline constexpr size_t upper_bound(size_t num)
{
    static_assert(is_2_n<MULTIPLE>::val);
    return (num + MULTIPLE - 1) & (~(MULTIPLE - 1));
}

template <typename T>
using Small_vector_as_vec = Small_vector<T, upper_bound<4>(sizeof(Vector<T>))>;
}  // namespace pcc

#endif