#pragma once
#ifndef PCC_CONFIG_H_PCC_
#define PCC_CONFIG_H_PCC_

#include <cassert>
#include <cstdint>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @brief define HAS_BOOST if we can use the boost library
 */
#define HAS_BOOST
#undef HAS_BOOST

namespace pcc
{
using UInt = uint32_t;
using Int = int32_t;
using Char = char;
using UChar = unsigned char;

inline bool is_digit(Char c) { return c >= '0' && c <= '9'; }
inline bool is_alpha(Char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
inline UInt char_to_digit(Char c) { return c - '0'; }
}  // namespace pcc

#endif