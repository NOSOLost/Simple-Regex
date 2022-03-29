#pragma once
#ifndef FA_STATUS_H_PCC_
#define FA_STATUS_H_PCC_

#include "pcc_config.h"

namespace pcc
{
/**
 * @brief the status of FA
 */
namespace fa_status
{
using Status_t = UInt;
static_assert(sizeof(Status_t) >= sizeof(Char) * 2 && sizeof(Status_t) >= 2, "The size of Status_t is too small");

static constexpr Status_t CHAR_AMOUNT = (1 << sizeof(Char) * 8);
static constexpr Status_t ACTION_AMOUNT = 256;
static constexpr Status_t SIGN_AMOUNT = 256;

/**
 * @brief the status range that represent char
 */
static constexpr Status_t CHAR_END = Status_t(-1);
static constexpr Status_t CHAR_BEG = CHAR_END - (CHAR_AMOUNT - 1);

/**
 * @brief the status range that represent action
 */
static constexpr Status_t ACTION_END = CHAR_BEG - 1;
static constexpr Status_t ACTION_BEG = ACTION_END - (ACTION_AMOUNT - 1);

/**
 * @brief the status range that represent sign
 */
static constexpr Status_t SIGN_END = ACTION_BEG - 1;
static constexpr Status_t SIGN_BEG = SIGN_END - (SIGN_AMOUNT - 1);

static constexpr Status_t NOT_INDEX_BEG = SIGN_BEG;
static constexpr Status_t MAX_STATUS_INDEX = SIGN_BEG - 1;

/**
 * @brief the status range that represent special sign (the range is from SIGN_BEG to SIGN_END at most)
 */
static constexpr Status_t SIGN_FAILURE = SIGN_BEG + 1;          // lexical analysis failure
static constexpr Status_t SIGN_DOLLER = SIGN_BEG + 2;           // $, the end
static constexpr Status_t SIGN_LEFT_BRACKET = SIGN_BEG + 3;     // (
static constexpr Status_t SIGN_RIGHT_BRACKET = SIGN_BEG + 4;    // )
static constexpr Status_t SIGN_ASTERISK = SIGN_BEG + 5;         // *
static constexpr Status_t SIGN_OR = SIGN_BEG + 6;               // |
static constexpr Status_t SIGN_ADD = SIGN_BEG + 7;              // +
static constexpr Status_t SIGN_QUES = SIGN_BEG + 8;             // ?
static constexpr Status_t SIGN_DOT = SIGN_BEG + 9;              // .
static constexpr Status_t SIGN_LEFT_BRACE = SIGN_BEG + 10;      // {
static constexpr Status_t SIGN_RIGHT_BRACE = SIGN_BEG + 11;     // }
static constexpr Status_t SIGN_COMMA = SIGN_BEG + 12;           // ,
static constexpr Status_t SIGN_LEFT_SQUBRACE = SIGN_BEG + 13;   // [
static constexpr Status_t SIGN_RIGHT_SQUBRACE = SIGN_BEG + 14;  // ]
static constexpr Status_t SIGN_MINUS = SIGN_BEG + 15;           // -
static constexpr Status_t SIGN_XOR = SIGN_BEG + 16;             // ^

/**
 * @param status the status to be identify
 * @return true if the status is in the range
 */
static constexpr bool status_means_char(Status_t status) { return status >= CHAR_BEG && status <= CHAR_END; }
static constexpr bool status_means_action(Status_t status) { return status >= ACTION_BEG && status <= ACTION_END; }
static constexpr bool status_means_sign(Status_t status) { return status >= SIGN_BEG && status <= SIGN_END; }
static constexpr bool status_means_status(Status_t status) { return status < SIGN_BEG; }

/**
 * @param status the status to be change
 * @return the target that the status represent
 * 
 * for the function char_to_status, turn the Char to the UChar to prevent the negtive char
 */
static constexpr Status_t char_to_status(Char c) { return UChar(c) + CHAR_BEG; }
static constexpr Char status_to_char(Status_t status) { return Char(status - CHAR_BEG); }
static constexpr Status_t action_index_to_status(Status_t action_index) { return action_index + ACTION_BEG; }
static constexpr Status_t status_to_action_index(Status_t status) { return status - ACTION_BEG; }
}  // namespace fa_status
}  // namespace pcc

#endif  // FA_STATUS_H_PCC_