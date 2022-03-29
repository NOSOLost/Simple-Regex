#pragma once
#ifndef REGEX_LEXER_H_PCC_
#define REGEX_LEXER_H_PCC_

#include "fa_status.h"
#include "pcc_config.h"
#include "stream_buffer.h"

namespace pcc
{
using namespace fa_status;

/**
 * @brief the lexer of regex, read input from the specified stream 
 */
template <typename Char_t, typename Stream>
class Regex_lexer
{
    static_assert(is_same_v<Char_t, Char>, "Regex_lexer only support type of Char");

public:
    template <typename NStream>
    Regex_lexer(Char_t* beg, Char_t* end, NStream& stream) : buff(beg, end, stream)
    {
        buff.fill_buff();
    }

    /**
     * @brief get next token from specified stream
     * 
     * @return return the token whose type is Status_t
     *         return SIGN_FAIL when lexer meet wrong input
     */
    Status_t next_token()
    {
        using namespace fa_status;
        Char_t nextc;
        while (true) {
            nextc = buff.next();
            assert(nextc == EOF || 32 <= nextc <= 127);
            switch (nextc) {
                case '(':
                    return SIGN_LEFT_BRACKET;
                case ')':
                    return SIGN_RIGHT_BRACKET;
                case '*':
                    return SIGN_ASTERISK;
                case '|':
                    return SIGN_OR;
                case '+':
                    return SIGN_ADD;
                case '?':
                    return SIGN_QUES;
                case '.':
                    return SIGN_DOT;
                case '{':
                    return SIGN_LEFT_BRACE;
                case '}':
                    return SIGN_RIGHT_BRACE;
                case ',':
                    return SIGN_COMMA;
                case '[':
                    return SIGN_LEFT_SQUBRACE;
                case ']':
                    return SIGN_RIGHT_SQUBRACE;
                case '-':
                    return SIGN_MINUS;
                case '^':
                    return SIGN_XOR;
                case '\\':
                    nextc = buff.next();
                    if (nextc == EOF) {
                        if (!try_read())
                            return SIGN_FAILURE;
                        nextc = buff.next();
                    }
                    nextc = solve_escape_char(nextc);
                    return nextc == EOF ? SIGN_FAILURE : char_to_status(nextc);
                case EOF:
                    if (!try_read())
                        return SIGN_DOLLER;
                    continue;
            }
            break;
        }
        return char_to_status(nextc);
    }

    static constexpr UInt BUFF_SIZE = 1 << 8;

private:
    Char_t solve_escape_char(Char_t c)
    {
        switch (c) {
            case '\\':
                return '\\';
            case '(':
            case ')':
            case '*':
            case '|':
            case '+':
            case '?':
            case '.':
            case '{':
            case '}':
            case ',':
            case '[':
            case ']':
            case '-':
            case '^':
                return c;
        }

        return EOF;
    }

    bool try_read()
    {
        if (!buff.has_stream())
            return false;
        buff.fill_buff();
        return true;
    }

    Stream_buff<Char_t, BUFF_SIZE, Stream> buff;
};
}  // namespace pcc

#endif  // REGEX_LEXER_H_PCC_