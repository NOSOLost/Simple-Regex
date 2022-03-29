#pragma once
#ifndef REGEX_H_PCC_
#define REGEX_H_PCC_

#include <cstdio>
#include <cstring>
#include <exception>
#include <functional>
#include <string>
#include <utility>

#include "fa_status.h"
#include "pcc_config.h"
#include "pcc_template.h"
#include "regex_lexer.h"
#ifdef DEBUG
#include "test_tools.h"
#endif

namespace pcc
{
using namespace fa_status;

/**
 * @brief A set contains (1) the begin status and end status of a sub regex represented by NFA,
 *                       (2) the node_type of this node
 */
struct NFA_node_set {
    static constexpr UInt SINGEL_CHAR = 1;
    static constexpr UInt MID_SEQUENCE = 1 << 1;
    static constexpr UInt COMPLETE_SEQ = 1 << 2;

    static constexpr UInt CHAR_N_CHAR = SINGEL_CHAR;
    static constexpr UInt CHAR_N_MIDSEQ = SINGEL_CHAR | MID_SEQUENCE;
    static constexpr UInt CHAR_N_COMPSEQ = SINGEL_CHAR | COMPLETE_SEQ;
    static constexpr UInt MIDSEQ_N_MIDSEQ = MID_SEQUENCE;
    static constexpr UInt MIDSEQ_N_COMPSEQ = MID_SEQUENCE | COMPLETE_SEQ;
    static constexpr UInt COMPSEQ_N_COMPSEQ = COMPLETE_SEQ;

    NFA_node_set(UInt nt, Status_t s1, Status_t s2) : node_type(nt), elems(s1, s2) {}

    std::string to_string() const
    {
        switch (node_type) {
            case NFA_node_set::SINGEL_CHAR:
                return std::string("Char : ") + status_to_char(elems.first);
            case NFA_node_set::MID_SEQUENCE:
                return std::string("Mid_Seq : ") + std::to_string(elems.first) + " " + std::to_string(elems.second);
            case NFA_node_set::COMPLETE_SEQ:
                return std::string("Complete_Seq : ") + std::to_string(elems.first) + " " +
                       std::to_string(elems.second);
        }
        return std::string("ERROR");
    }

    /**
     * @brief the node_type of the node
     *
     *
     * For example
     * (1) the char 'a', node_type = SIGEL_CHAR
     *
     * (2) a regex: (ab)*
     *                     __________________
     *                    |                 |
     *                    |                 V
     *    its NFA status: 0 -(a)-> 1 -(b)-> 2
     *
     *    its node_type: MID_SEQUENSE
     *
     * (3) a regex: (ab|c*)
     *                            __________________
     *                           |                 |
     *                           |                 V
     *    its NFA status:   ---> 0 -(a)-> 1 -(b)-> 2 ---
     *                      |                          |
     *                      |                          V
     *                      5     __________           6
     *                      |    |         |           ^
     *                      |    |        V            |
     *    its NFA status:   ---> 3 -(c)-> 4-------------
     *
     *
     *    its node_type: COMPLETE_SEQ
     */
    UInt node_type;

    /**
     * the begin status and end status of a sub regex represented by NFA,
     *
     *
     * For example, a regex: (ab)*
     *                               __________________
     *                              |                 |
     *                              |                 V
     *              its NFA status: 0 -(a)-> 1 -(b)-> 2
     *
     *              the NFA_node_set: NFA_node_set(0, 2)
     */
    std::pair<Status_t, Status_t> elems;
};

/**
 * @brief the node of the nfa
 */
template <typename Char_t>
struct NFA_node {
    using SmallVec = Small_vector_as_vec<Status_t>;

    NFA_node() = default;
    NFA_node(SmallVec&& v, Hash_map<Char_t, Status_t>&& m)
        : empty_trans(std::move(v)), trans(std::move(m)), node_type(COMMON_NODE)
    {
    }

    void set_node_type(Status_t s) { node_type = s; }

    Status_t get_node_type() { return node_type; }

    void into_all_alpha_node(Status_t s)
    {
        trans.insert({ 0, s });
        node_type = ALL_ALPHA_NODE;
    }

    void into_xor_alpha_node(Status_t s)
    {
        trans.insert({ 0, s });
        node_type = XOR_ALPHA_NODE;
    }

    void add_trans(Char_t c, Status_t s) { trans.insert({ c, s }); }

    void add_empty_trans(Status_t s) { empty_trans.push_back(s); }

    bool has_empty_trans() const { return !empty_trans.empty(); }

    bool has_trans() const { return !trans.empty(); }

    SmallVec& get_empty_trans() { return empty_trans; }

    Hash_map<Char_t, Status_t>& get_trans() { return trans; }

    std::pair<bool, Status_t> trans_to(Char_t c)
    {
        typename Hash_map<Char_t, Status_t>::iterator iter;
        switch (node_type) {
            case COMMON_NODE:
                iter = trans.find(c);
                return { iter != trans.end(), iter != trans.end() ? iter->second : 0 };
            case ALL_ALPHA_NODE:
                return { true, trans.begin()->second };
            case XOR_ALPHA_NODE:
                iter = trans.find(c);
                return { iter == trans.end(), iter == trans.end() ? trans.begin()->second : 0 };
        }
        assert(false);
        exit(1);
    }

    void show_empty_trans()
    {
#ifdef DEBUG
        pcc_test::show_container(get_empty_trans());
#endif
    }

    void show_trans()
    {
#ifdef DEBUG
        for (auto now = trans.begin(); now != trans.end(); ++now)
            pcc_test::print("[ ", now->first, " ", now->second, " ]");
        if (node_type == ALL_ALPHA_NODE)
            pcc_test::print(" <.>node");
        if (node_type == XOR_ALPHA_NODE)
            pcc_test::print(" <^>node");
#endif
    }

    static constexpr UInt COMMON_NODE = 0;
    static constexpr UInt ALL_ALPHA_NODE = 1;
    static constexpr UInt XOR_ALPHA_NODE = 2;

private:
    /**
     * @brief all the trans of the node of the nfa
     *
     *
     * For example, a regex: (ab)*
     *                               __________________
     *                              |                 |
     *                              |                 V
     *              its NFA status: 0 -(a)-> 1 -(b)-> 2
     *
     *              the NFA_node of the status 0: empty_trans = [2], trans = [[a, 1]]
     *              the NFA_node of the status 1: empty_trans = [], trans = [[b, 2]]
     *              the NFA_node of the status 2: empty_trans = [], trans = []
     *
     *
     * for the empty_trans, it usually has a few transformations,
     * so we use small_vector to reduce the memory usage and avoid cache miss
     */
    UInt node_type = COMMON_NODE;
    SmallVec empty_trans;
    Hash_map<Char_t, Status_t> trans;
};

/**
 * @brief the template of the Regex
 *
 * @tparam Char_t the char type that the regex will handle, only support the type char for now
 */
template <typename Char_t>
class Basic_regex
{
    static_assert(is_same_v<Char_t, Char>, "Regex only support the type Char");

    template <typename _Char_t, typename Identi_action, typename Return_type>
    friend class Basic_regex_match;

public:
    Basic_regex() = default;

    Basic_regex(const Basic_regex& other) = default;

    Basic_regex(Basic_regex&& other) = default;

    template <typename Stream>
    Basic_regex(Stream& stream) : Basic_regex()
    {
        if (!regenetare_regex(stream))
            throw std::logic_error("Wrong regex");
    }

    Basic_regex(const Char_t* regex) : Basic_regex()
    {
        if (!regenetare_regex(regex))
            throw std::logic_error("Wrong regex");
    }

    Basic_regex& operator=(const Basic_regex& other) = default;

    Basic_regex& operator=(Basic_regex&& other) = default;

    ~Basic_regex() = default;

    template <typename Stream>
    bool regenetare_regex(Stream& stream)
    {
        clear();
        Vector<NFA_node_set> cache_stack;
        if (!generate_nfa(stream, cache_stack)) {
            clear();
            return false;
        }
        assert(cache_stack.size() == 1);
        start_status = cache_stack.back().elems.first;
        accept_state.push_back(cache_stack.back().elems.second);

        return true;
    }

    bool regenetare_regex(const Char_t* regex)
    {
        std::stringstream regex_stream(regex);
        return regenetare_regex(regex_stream);
    }

    void clear()
    {
        nfa.clear();
        accept_state.clear();
    }

private:
    template <typename Stream>
    bool generate_nfa(Stream& stream, Vector<NFA_node_set>& cache_stack)
    {
        Vector<Status_t> status;
        Regex_lexer<Char_t, Stream> lexer(lexer_buff_memory, lexer_buff_memory + LEXER_BUFF_SIZE, stream);

        status.push_back(pred_table_begin_status());
        Status_t token = lexer.next_token();
        if (lex_analy_fail(token))
            return false;

        while (!status.empty()) {
            Status_t now_st = status.back();
            status.pop_back();

            if (status_means_status(now_st)) {
                const Vector<Status_t>* product = predicion_table[now_st].get(token);
                if (product_fail(product))
                    return false;
                status.insert(status.end(), product->rbegin(), product->rend());
            } else if (status_means_sign(now_st)) {
                if (now_st != token || lex_analy_fail(token))
                    return false;
                token = lexer.next_token();
            } else if (status_means_action(now_st)) {
                execute_action(now_st, cache_stack, token, lexer);
            } else {
                assert(false);
            }
        }

        return token == SIGN_DOLLER;
    }

    static bool product_fail(const Vector<Status_t>* vec) { return vec == &Production_FAILURE; }

    static bool lex_analy_fail(Status_t status) { return status == SIGN_FAILURE; }

    void execute_action(Status_t act_status, Vector<NFA_node_set>& stack, Status_t& token,
                        Regex_lexer<Char_t, std::stringstream>& lexer)
    {
        switch (act_status) {
            case ACTION_UNION:
                act_union(stack);
                return;
            case ACTION_OR:
                act_or(stack);
                return;
            case ACTION_REP:
                act_rep(stack);
                return;
            case ACTION_ALPHA:
                act_alpha(stack, token, lexer);
                return;
            case ACTION_ONE_OR:
                act_one_or(stack);
                return;
            case ACTION_ZERO_ONE:
                act_zero_one(stack);
                return;
            case ACTION_ANY_ALPHA:
                act_any_alpha(stack, token, lexer);
                return;
            case ACTION_REP_FOR:
                act_rep_for(stack, token, lexer);
                return;
            case ACTION_RANGE:
                act_range(stack, token, lexer);
                return;
        }
        assert(false);
    }

    /**
     * @brief union two NFA_node_set
     *
     *
     * For example,
     * (1) NFA_node_set(SIGEL_CHAR, status_to_char('a'), ?), NFA_node_set(SIGEL_CHAR, status_to_char('b'), ?)
     *     the next NFA status that can be use is 5 (1-4 have been used)
     *     will be union to 5 -(a)-> 6 -(b)-> 7
     *     and leave the NFA_node_set(MID_SEQENCE) on stack
     *
     * (2) NFA_node_set(COMPLETE_SEQ, 0, 5), NFA_node_set(COMPLETE_SEQ, 6, 10)
     *     will be union as 0 -> ... -> 5 -> 6 -> ... -> 10
     *     and leave the NFA_node_set(COMPLETE_SEQENCE) on stack
     */
    void act_union(Vector<NFA_node_set>& stack)
    {
        NFA_node_set* node_1 = &stack[stack.size() - 2];
        NFA_node_set* node_2 = &stack.back();
        Status_t now_status = nfa.size();
        typename Vector<NFA_node<Char_t>>::iterator iter;
        Vector<NFA_node_set>::iterator mid_iter;

        switch (node_1->node_type | node_2->node_type) {
            case NFA_node_set::CHAR_N_CHAR:
                nfa.resize(nfa.size() + 3);
                iter = nfa.end() - 3;
                iter->add_trans(status_to_char(node_1->elems.first), now_status + 1);
                ++iter;
                iter->add_trans(status_to_char(node_2->elems.first), now_status + 2);
                stack.pop_back();
                stack.back().node_type = NFA_node_set::MID_SEQUENCE;
                stack.back().elems.first = now_status;
                stack.back().elems.second = now_status + 2;
                break;

            case NFA_node_set::CHAR_N_MIDSEQ:
            case NFA_node_set::CHAR_N_COMPSEQ:
                nfa.resize(nfa.size() + 1);
                if (node_1->node_type == NFA_node_set::SINGEL_CHAR) {
                    nfa.back().add_trans(status_to_char(node_1->elems.first), node_2->elems.first);
                    now_status = node_2->elems.second;
                    stack.pop_back();
                    stack.back().node_type = NFA_node_set::MID_SEQUENCE;
                    stack.back().elems.first = nfa.size() - 1;
                    stack.back().elems.second = now_status;
                } else {
                    nfa[node_1->elems.second].add_trans(status_to_char(node_2->elems.first), nfa.size() - 1);
                    stack.pop_back();
                    stack.back().node_type = NFA_node_set::MID_SEQUENCE;
                    stack.back().elems.second = nfa.size() - 1;
                }
                break;

            case NFA_node_set::MIDSEQ_N_MIDSEQ:
            case NFA_node_set::MIDSEQ_N_COMPSEQ:
            case NFA_node_set::COMPSEQ_N_COMPSEQ:
                nfa[node_1->elems.second].add_empty_trans(node_2->elems.first);
                mid_iter = stack.end() - 2;
                if (mid_iter->node_type == NFA_node_set::COMPLETE_SEQ &&
                    (mid_iter + 1)->node_type == NFA_node_set::COMPLETE_SEQ)
                    mid_iter->node_type = NFA_node_set::COMPLETE_SEQ;
                now_status = stack.back().elems.second;
                stack.pop_back();
                stack.back().elems.second = now_status;
                break;
        }

        debug_show(stack, "union end");
    }

    void act_or(Vector<NFA_node_set>& stack)
    {
        NFA_node_set* node_1 = &stack[stack.size() - 2];
        NFA_node_set* node_2 = &stack.back();
        Status_t now_status = nfa.size();
        typename Vector<NFA_node<Char_t>>::iterator iter;
        Vector<NFA_node_set>::iterator mid_iter;

        switch (node_1->node_type | node_2->node_type) {
            case NFA_node_set::CHAR_N_CHAR:
                nfa.resize(nfa.size() + 2);
                iter = nfa.end() - 2;
                iter->add_trans(status_to_char(node_1->elems.first), now_status + 1);
                iter->add_trans(status_to_char(node_2->elems.first), now_status + 1);
                stack.pop_back();
                stack.back().node_type = NFA_node_set::MID_SEQUENCE;
                stack.back().elems.first = now_status;
                stack.back().elems.second = now_status + 1;
                break;

            case NFA_node_set::CHAR_N_MIDSEQ:
                if (node_1->node_type == NFA_node_set::SINGEL_CHAR)
                    or_char_midseq(node_1, node_2, stack);
                else
                    or_char_midseq(node_2, node_1, stack);
                stack.pop_back();
                stack.back().node_type = NFA_node_set::MID_SEQUENCE;
                stack.back().elems.first = now_status;
                stack.back().elems.second = now_status + 1;
                break;

            case NFA_node_set::CHAR_N_COMPSEQ:
                if (node_1->node_type == NFA_node_set::SINGEL_CHAR) {
                    or_char_compseq(node_1, node_2, stack);
                    stack.erase(stack.end() - 2);
                } else {
                    or_char_compseq(node_2, node_1, stack);
                    stack.pop_back();
                }
                stack.back().node_type = NFA_node_set::MID_SEQUENCE;
                break;

            case NFA_node_set::MIDSEQ_N_MIDSEQ:
                nfa.resize(nfa.size() + 2);
                iter = nfa.end() - 2;
                iter->add_empty_trans(node_1->elems.first);
                iter->add_empty_trans(node_2->elems.first);
                nfa[node_1->elems.second].add_empty_trans(now_status + 1);
                nfa[node_2->elems.second].add_empty_trans(now_status + 1);
                stack.pop_back();
                stack.back().node_type = NFA_node_set::COMPLETE_SEQ;
                stack.back().elems.first = now_status;
                stack.back().elems.second = now_status + 1;
                break;

            case NFA_node_set::MIDSEQ_N_COMPSEQ:
                if (node_1->node_type == NFA_node_set::MID_SEQUENCE) {
                    or_midseq_compseq(node_1, node_2, stack);
                    stack.erase(stack.end() - 2);
                } else {
                    or_midseq_compseq(node_2, node_1, stack);
                    stack.pop_back();
                }
                stack.back().node_type = NFA_node_set::COMPLETE_SEQ;
                break;

            case NFA_node_set::COMPSEQ_N_COMPSEQ:
                nfa[node_1->elems.first].add_empty_trans(node_2->elems.first);
                nfa[node_2->elems.second].add_empty_trans(node_1->elems.second);
                stack.pop_back();
                break;
        }

        debug_show(stack, "|, or end");
    }

    void or_char_midseq(NFA_node_set* node1, NFA_node_set* node2, Vector<NFA_node_set>& stack)
    {
        Status_t now_status = nfa.size();
        nfa.resize(nfa.size() + 2);
        auto iter = nfa.end() - 2;
        iter->add_trans(status_to_char(node1->elems.first), now_status + 1);
        iter->add_empty_trans(node2->elems.first);
        nfa[node2->elems.second].add_empty_trans(now_status + 1);
    }

    void or_char_compseq(NFA_node_set* node1, NFA_node_set* node2, Vector<NFA_node_set>& stack)
    {
        nfa[node2->elems.first].add_trans(status_to_char(node1->elems.first), node2->elems.second);
    }

    void or_midseq_compseq(NFA_node_set* node1, NFA_node_set* node2, Vector<NFA_node_set>& stack)
    {
        nfa[node2->elems.first].add_empty_trans(node1->elems.first);
        nfa[node1->elems.second].add_empty_trans(node2->elems.second);
    }

    void act_alpha(Vector<NFA_node_set>& stack, Status_t& token, Regex_lexer<Char_t, std::stringstream>& lexer)
    {
        stack.push_back(NFA_node_set(NFA_node_set::SINGEL_CHAR, token, 0));
        token = lexer.next_token();

        debug_show(stack, "push alpha end");
    }

    static constexpr UInt REPEAT_REP = 0;
    static constexpr UInt REPEAT_ONE_OR = 1;
    static constexpr UInt REPEAT_ZERO_ONE = 2;

    void act_rep(Vector<NFA_node_set>& stack)
    {
        repeat_action_aux<REPEAT_REP>(stack);
        debug_show(stack, "*, rep end");
    }

    void act_one_or(Vector<NFA_node_set>& stack)
    {
        repeat_action_aux<REPEAT_ONE_OR>(stack);
        debug_show(stack, "+, one_or end");
    }

    void act_zero_one(Vector<NFA_node_set>& stack)
    {
        repeat_action_aux<REPEAT_ZERO_ONE>(stack);
        debug_show(stack, "?, zero_one end");
    }

    template <UInt REPEAT_TYPE>
    void repeat_action_aux(Vector<NFA_node_set>& stack)
    {
        if (stack.back().node_type == NFA_node_set::SINGEL_CHAR) {
            Status_t now_status = nfa.size();
            nfa.resize(nfa.size() + 2);
            typename Vector<NFA_node<Char_t>>::iterator iter = nfa.end() - 2;
            iter->add_trans(status_to_char(stack.back().elems.first), now_status + 1);
            if constexpr (REPEAT_TYPE == REPEAT_ZERO_ONE || REPEAT_TYPE == REPEAT_REP) {
                iter->add_empty_trans(now_status + 1);
            }
            ++iter;
            if constexpr (REPEAT_TYPE == REPEAT_ONE_OR || REPEAT_TYPE == REPEAT_REP) {
                iter->add_empty_trans(now_status);
            }
            stack.back().elems.first = now_status;
            stack.back().elems.second = now_status + 1;
        } else {
            Status_t start = stack.back().elems.first;
            Status_t end = stack.back().elems.second;
            if constexpr (REPEAT_TYPE == REPEAT_ZERO_ONE || REPEAT_TYPE == REPEAT_REP) {
                nfa[start].add_empty_trans(end);
            }
            if constexpr (REPEAT_TYPE == REPEAT_ONE_OR || REPEAT_TYPE == REPEAT_REP) {
                nfa[end].add_empty_trans(start);
            }
        }
        stack.back().node_type = NFA_node_set::MID_SEQUENCE;
    }

    void act_any_alpha(Vector<NFA_node_set>& stack, Status_t& token, Regex_lexer<Char_t, std::stringstream>& lexer)
    {
        assert(token == SIGN_DOT);
        Status_t now_status = nfa.size();
        nfa.resize(nfa.size() + 2);
        auto iter = nfa.end() - 2;
        iter->into_all_alpha_node(now_status + 1);
        stack.push_back(NFA_node_set(NFA_node_set::MID_SEQUENCE, now_status, now_status + 1));
        token = lexer.next_token();

        debug_show(stack, ". any_alpha end");
    }

    void act_rep_for(Vector<NFA_node_set>& stack, Status_t& token, Regex_lexer<Char_t, std::stringstream>& lexer)
    {
        assert(token == SIGN_LEFT_BRACE);
        Status_t nums[2] = { 0, 0 };
        int meet_2nd = parse_nums(nums, token, lexer);
        if (token == SIGN_FAILURE)
            return;
        token = lexer.next_token();
        if (nums[0] == 0 && nums[1] == 1) {
            act_zero_one(stack);
            return;
        }

        if (!meet_2nd) {
            if (nums[0] == 0) {
                act_rep(stack);
                return;
            } else if (nums[0] == 1) {
                act_one_or(stack);
                return;
            }
            nums[1] = nums[0];
        }

        Status_t beg = nfa.size();
        auto& rep_range = stack.back();
        Char_t c;
        switch (rep_range.node_type) {
            case NFA_node_set::SINGEL_CHAR:
                c = status_to_char(rep_range.elems.first);
                nfa.resize(nfa.size() + nums[1] + 1);
                for (Status_t i = 0; i != nums[1]; ++i)
                    nfa[beg + i].add_trans(c, beg + i + 1);
                if (meet_2nd) {
                    for (Status_t i = beg + nums[0]; i != nfa.size() - 1; ++i)
                        nfa[i].add_empty_trans(nfa.size() - 1);
                } else {
                    nfa[beg + nums[0]].add_empty_trans(beg + nums[0] - 1);
                }
                rep_range.node_type = NFA_node_set::MID_SEQUENCE;
                rep_range.elems.first = beg;
                rep_range.elems.second = nfa.size() - 1;
                debug_show(stack, "{,}alpha");
                return;
            case NFA_node_set::MID_SEQUENCE:
                nfa.resize(nfa.size() + 2);
                nfa[beg].add_empty_trans(rep_range.elems.first);
                nfa[rep_range.elems.second].add_empty_trans(beg + 1);
                rep_range.elems.first = beg;
                rep_range.elems.second = beg + 1;
        }

        debug_show(stack, "{,} mid_handle");

        const Status_t new_status_beg = nfa.size();
        const Status_t expand_times = nums[1] - 2;
        Status_t new_end_ind;
        const Status_t new_status_size = generate_new_status(rep_range, new_end_ind);
        debug_show(stack, "{,} new_status");

        expand_other(new_status_beg, new_status_size, expand_times, rep_range);
        debug_show(stack, "{,} expand_other");

        link_all_status(new_status_beg, new_status_size, new_end_ind, rep_range);

        if (nums[0] != nums[1]) {
            if (meet_2nd) {
                generate_rep_for_trans(new_status_beg, new_status_size, new_end_ind, nums, rep_range);
                rep_range.node_type == NFA_node_set::COMPLETE_SEQ;
            } else {
                Status_t final_beg = new_status_beg + new_status_size * (nums[1] - 2);
                Status_t all_status_end = final_beg + new_end_ind;
                nfa[all_status_end].add_empty_trans(final_beg);
                rep_range.node_type == NFA_node_set::MID_SEQUENCE;
            }
        }

        rep_range.elems.second = new_status_beg + new_end_ind + expand_times * new_status_size;

        debug_show(stack, "{,} rep_for end");
    }

    int parse_nums(Status_t nums[2], Status_t& token, Regex_lexer<Char_t, std::stringstream>& lexer)
    {
        int meet_2nd;
        for (int i = 0; i != 2; ++i) {
            meet_2nd = 0;
            while (true) {
                token = lexer.next_token();
                if (token == (i == 0 ? SIGN_COMMA : SIGN_RIGHT_BRACE))
                    break;
                if (!status_means_char(token) || !is_digit(status_to_char(token))) {
                    token = SIGN_FAILURE;
                    return 0;
                }
                meet_2nd = 1;
                nums[i] = nums[i] * 10 + char_to_digit(status_to_char(token));
            }
        }
        if (meet_2nd && nums[0] > nums[1] && (nums[0] != 0 && nums[1] != 0))
            token = SIGN_FAILURE;

        return meet_2nd;
    }

    Status_t generate_new_status(NFA_node_set rep_range, Status_t& new_end_ind)
    {
        static constexpr UInt array_map_size = 64;
        if (nfa.size() > array_map_size)
            return generate_new_status_large(rep_range, new_end_ind);

        Vector<Status_t> added_old_status;
        Status_t old_to_new_status[array_map_size];
        memset(old_to_new_status, (unsigned char)(-1), sizeof(old_to_new_status));
        const Status_t new_status_beg = nfa.size();
        Status_t new_status_ind = 0;
        added_old_status.push_back(rep_range.elems.first);
        nfa.resize(nfa.size() + 1);
        nfa.back().set_node_type(nfa[rep_range.elems.first].get_node_type());

        while (new_status_ind != added_old_status.size()) {
            Status_t now_copy_status = added_old_status[new_status_ind];
            Status_t now_new_status = new_status_beg + new_status_ind;

            int size = nfa[now_copy_status].get_empty_trans().size();
            for (int i = 0; i != size; ++i) {
                Status_t emp_trans_to_status = nfa[now_copy_status].get_empty_trans()[i];
                if (old_to_new_status[emp_trans_to_status] == UInt(-1)) {
                    old_to_new_status[emp_trans_to_status] = nfa.size();
                    nfa.resize(nfa.size() + 1);
                    nfa.back().set_node_type(nfa[emp_trans_to_status].get_node_type());
                    added_old_status.push_back(emp_trans_to_status);
                }
                nfa[now_new_status].add_empty_trans(old_to_new_status[emp_trans_to_status]);
            }

            for (auto kv : nfa[now_copy_status].get_trans()) {
                if (old_to_new_status[kv.second] == UInt(-1)) {
                    old_to_new_status[kv.second] = nfa.size();
                    nfa.resize(nfa.size() + 1);
                    nfa.back().set_node_type(nfa[kv.second].get_node_type());
                    added_old_status.push_back(kv.second);
                }
                nfa[now_new_status].add_trans(kv.first, old_to_new_status[kv.second]);
            }
            ++new_status_ind;
        }
        new_end_ind = old_to_new_status[rep_range.elems.second] - new_status_beg;

        return new_status_ind;
    }

    Status_t generate_new_status_large(NFA_node_set rep_range, Status_t& new_end_ind)
    {
        Vector<Status_t> added_old_status;
        Hash_map<Status_t, Status_t> old_to_new_status;
        const Status_t new_status_beg = nfa.size();
        Status_t new_status_ind = 0;
        added_old_status.push_back(rep_range.elems.first);
        nfa.resize(nfa.size() + 1);
        nfa.back().set_node_type(nfa[rep_range.elems.first].get_node_type());

        while (new_status_ind != added_old_status.size()) {
            Status_t now_copy_status = added_old_status[new_status_ind];
            Status_t now_new_status = new_status_beg + new_status_ind;

            int size = nfa[now_copy_status].get_empty_trans().size();
            for (int i = 0; i != size; ++i) {
                Status_t emp_trans_to_status = nfa[now_copy_status].get_empty_trans()[i];
                auto iter = old_to_new_status.find(emp_trans_to_status);

                if (iter == old_to_new_status.end()) {
                    iter = old_to_new_status.insert({ emp_trans_to_status, nfa.size() }).first;
                    nfa.resize(nfa.size() + 1);
                    nfa.back().set_node_type(nfa[emp_trans_to_status].get_node_type());
                    added_old_status.push_back(emp_trans_to_status);
                }
                nfa[now_new_status].add_empty_trans(iter->second);
            }

            for (auto kv : nfa[now_copy_status].get_trans()) {
                auto iter = old_to_new_status.find(kv.second);

                if (iter == old_to_new_status.end()) {
                    iter = old_to_new_status.insert({ kv.second, nfa.size() }).first;
                    nfa.resize(nfa.size() + 1);
                    nfa.back().set_node_type(nfa[kv.second].get_node_type());
                    added_old_status.push_back(kv.second);
                }
                nfa[now_new_status].add_trans(kv.first, iter->second);
            }
            ++new_status_ind;
        }
        new_end_ind = old_to_new_status[rep_range.elems.second] - new_status_beg;

        return new_status_ind;
    }

    void expand_other(Status_t new_status_beg, Status_t new_status_size, int expand_times, NFA_node_set rep_range)
    {
        nfa.resize(nfa.size() + expand_times * new_status_size);
        for (Status_t expand_status_beg = new_status_beg + new_status_size; expand_times-- != 0;
             expand_status_beg += new_status_size) {
            Status_t expand_status_ind = 0;
            for (Status_t new_status_ind = 0; new_status_ind != new_status_size; ++new_status_ind) {
                Status_t now_expand_status = expand_status_beg + expand_status_ind;
                Status_t now_copy_status = new_status_beg + new_status_ind;

                nfa[now_expand_status].set_node_type(nfa[now_copy_status].get_node_type());

                auto& empty_trans = nfa[now_expand_status].get_empty_trans();
                empty_trans = nfa[new_status_beg + new_status_ind].get_empty_trans();
                std::for_each(empty_trans.begin(), empty_trans.end(),
                              [=](Status_t& i) { i = expand_status_beg + i - new_status_beg; });
                auto& trans = nfa[now_expand_status].get_trans();
                trans = nfa[now_copy_status].get_trans();
                std::for_each(trans.begin(), trans.end(), [=](std::pair<const Char_t, Status_t>& kv) {
                    kv.second = expand_status_beg + kv.second - new_status_beg;
                });
                ++expand_status_ind;
            }
        }
    }

    void link_all_status(Status_t new_status_beg, Status_t new_status_size, Status_t new_end_ind,
                         NFA_node_set rep_range)
    {
        nfa[rep_range.elems.second].add_empty_trans(new_status_beg);
        for (Status_t end = new_status_beg + new_end_ind, next_beg = new_status_beg + new_status_size;
             next_beg < nfa.size(); end += new_status_size, next_beg += new_status_size) {
            nfa[end].add_empty_trans(next_beg);
        }
    }

    void generate_rep_for_trans(Status_t new_status_beg, Status_t new_status_size, Status_t new_end_ind, UInt nums[2],
                                NFA_node_set rep_range)
    {
        Status_t all_status_end = new_status_beg + new_end_ind + new_status_size * (nums[1] - 2);
        if (nums[0] == 0)
            nfa[rep_range.elems.first].add_empty_trans(all_status_end);
        for (Status_t beg = new_status_beg + new_status_size * (nums[0] - 1);
             beg != new_status_beg + new_status_size * (nums[1] - 1); beg += new_status_size)
            nfa[beg].add_empty_trans(all_status_end);
    }

    void act_range(Vector<NFA_node_set>& stack, Status_t& token, Regex_lexer<Char_t, std::stringstream>& lexer)
    {
        assert(token == SIGN_LEFT_SQUBRACE);
        Status_t now_status = nfa.size();
        nfa.resize(nfa.size() + 2);
        auto iter = nfa.end() - 2;
        token = lexer.next_token();
        if (token == SIGN_XOR) {
            iter->into_xor_alpha_node(now_status + 1);
            token = lexer.next_token();
        }

        do {
            if (!status_means_char(token)) {
                token = SIGN_FAILURE;
                return;
            }
            Char_t beg_char = status_to_char(token);
            iter->add_trans(beg_char, now_status + 1);
            token = lexer.next_token();
            if (token != SIGN_MINUS) {
                if (token == SIGN_RIGHT_BRACE)
                    break;
                continue;
            }

            token = lexer.next_token();
            Char_t last_char = status_to_char(token);
            if (beg_char >= last_char || !status_means_char(token)) {
                token = SIGN_FAILURE;
                return;
            }
            for (++beg_char; beg_char <= last_char; ++beg_char)
                iter->add_trans(beg_char, now_status + 1);
            token = lexer.next_token();
        } while (token != SIGN_RIGHT_SQUBRACE);
        stack.push_back(NFA_node_set(NFA_node_set::MID_SEQUENCE, now_status, now_status + 1));
        token = lexer.next_token();
    }

    static Status_t pred_table_begin_status() { return STATUS_E; }

    void debug_show(Vector<NFA_node_set>& stack, const Char_t* str)
    {
#ifdef DEBUG
        using namespace pcc_test;
        show_me();
        show_container_map(stack, [](const NFA_node_set& n) { return n.to_string(); }) << "\n" << str << "\n\n";
#endif
    }

    void show_me()
    {
#ifdef DEBUG
        using namespace pcc_test;
        println("NFA : ");
        if (nfa.empty()) {
            println("<empty NFA>");
            return;
        }

        int i = 0;
        println("<");
        for (auto& node : nfa) {
            print(i++, ": empty_trans : ");
            if (!node.has_empty_trans())
                print("empty");
            else
                node.show_empty_trans();

            print(", trans : ");
            if (!node.has_trans())
                print("empty");
            else
                node.show_trans();
            println();
        }
        println(">");
#endif
    }

    struct Regex_LL1_trans {
        Regex_LL1_trans() : alpha_trans(nullptr), trans() {}
        Regex_LL1_trans(const Regex_LL1_trans& r) = default;
        Regex_LL1_trans(Regex_LL1_trans&& r) = default;
        ~Regex_LL1_trans() = default;

        Regex_LL1_trans& operator=(const Regex_LL1_trans& r) = default;
        Regex_LL1_trans& operator=(Regex_LL1_trans&& r) = default;

        const Vector<Status_t>* get(Status_t status)
        {
            if (alpha_trans != nullptr && status_means_char(status))
                return alpha_trans;
            auto iter = trans.find(status);
            return iter != trans.end() ? iter->second : &Production_FAILURE;
        }

        const Vector<Status_t>* alpha_trans;
        Hash_map<Status_t, const Vector<Status_t>*> trans;
    };

    static Vector<Regex_LL1_trans> init_predicion_table()
    {
        using namespace fa_status;
        static const Vector<Status_t> E_to_T_En{ STATUS_T, STATUS_En };

        static const Vector<Status_t> En_to_s0or_T_f0union_En{ SIGN_OR, STATUS_T, ACTION_OR, STATUS_En };
        // static const Vector<Status_t> En_to_nop{};

        static const Vector<Status_t> T_to_T1_Tn{ STATUS_T1, STATUS_Tn };

        static const Vector<Status_t> Tn_to_T1_f0union_Tn{ STATUS_T1, ACTION_UNION, STATUS_Tn };
        // static const Vector<Status_t> Tn_to_nop{};

        static const Vector<Status_t> T1_to_F_R{ STATUS_F, STATUS_R };

        static const Vector<Status_t> R_to_s0asterisk_f0rep{ SIGN_ASTERISK, ACTION_REP };
        static const Vector<Status_t> R_to_s0add_f0oneor{ SIGN_ADD, ACTION_ONE_OR };
        static const Vector<Status_t> R_to_s0ques_f0zeroone{ SIGN_QUES, ACTION_ZERO_ONE };
        static const Vector<Status_t> R_to_f0repfor{ ACTION_REP_FOR };
        // static const Vector<Status_t> R_to_nop{};

        static const Vector<Status_t> F_to_s0lfbrack_E_s0rtbrack{ SIGN_LEFT_BRACKET, STATUS_E, SIGN_RIGHT_BRACKET };
        static const Vector<Status_t> F_to_f0alpha{ ACTION_ALPHA };
        static const Vector<Status_t> F_to_f0anyalpha{ ACTION_ANY_ALPHA };
        static const Vector<Status_t> F_to_f0range{ ACTION_RANGE };
        static const Vector<Status_t> ANY_to_nop{};

        Vector<Regex_LL1_trans> ptable(STATUS_NUM);
        ptable[STATUS_E].alpha_trans = &E_to_T_En;
        ptable[STATUS_E].trans.insert({ SIGN_LEFT_BRACKET, &E_to_T_En });
        ptable[STATUS_E].trans.insert({ SIGN_DOT, &E_to_T_En });
        ptable[STATUS_E].trans.insert({ SIGN_LEFT_SQUBRACE, &E_to_T_En });

        ptable[STATUS_En].alpha_trans = nullptr;
        ptable[STATUS_En].trans.insert({ SIGN_OR, &En_to_s0or_T_f0union_En });
        ptable[STATUS_En].trans.insert({ SIGN_RIGHT_BRACKET, &ANY_to_nop });
        ptable[STATUS_En].trans.insert({ SIGN_DOLLER, &ANY_to_nop });

        ptable[STATUS_T].alpha_trans = &T_to_T1_Tn;
        ptable[STATUS_T].trans.insert({ SIGN_LEFT_BRACKET, &T_to_T1_Tn });
        ptable[STATUS_T].trans.insert({ SIGN_DOT, &T_to_T1_Tn });
        ptable[STATUS_T].trans.insert({ SIGN_LEFT_SQUBRACE, &T_to_T1_Tn });

        ptable[STATUS_Tn].alpha_trans = &Tn_to_T1_f0union_Tn;
        ptable[STATUS_Tn].trans.insert({ SIGN_LEFT_BRACKET, &Tn_to_T1_f0union_Tn });
        ptable[STATUS_Tn].trans.insert({ SIGN_DOT, &Tn_to_T1_f0union_Tn });
        ptable[STATUS_Tn].trans.insert({ SIGN_LEFT_SQUBRACE, &Tn_to_T1_f0union_Tn });
        ptable[STATUS_Tn].trans.insert({ SIGN_OR, &ANY_to_nop });
        ptable[STATUS_Tn].trans.insert({ SIGN_RIGHT_BRACKET, &ANY_to_nop });
        ptable[STATUS_Tn].trans.insert({ SIGN_DOLLER, &ANY_to_nop });

        ptable[STATUS_T1].alpha_trans = &T1_to_F_R;
        ptable[STATUS_T1].trans.insert({ SIGN_LEFT_BRACKET, &T1_to_F_R });
        ptable[STATUS_T1].trans.insert({ SIGN_DOT, &T1_to_F_R });
        ptable[STATUS_T1].trans.insert({ SIGN_LEFT_SQUBRACE, &T1_to_F_R });

        ptable[STATUS_R].alpha_trans = &ANY_to_nop;
        ptable[STATUS_R].trans.insert({ SIGN_ASTERISK, &R_to_s0asterisk_f0rep });
        ptable[STATUS_R].trans.insert({ SIGN_ADD, &R_to_s0add_f0oneor });
        ptable[STATUS_R].trans.insert({ SIGN_QUES, &R_to_s0ques_f0zeroone });
        ptable[STATUS_R].trans.insert({ SIGN_LEFT_BRACE, &R_to_f0repfor });
        ptable[STATUS_R].trans.insert({ SIGN_OR, &ANY_to_nop });
        ptable[STATUS_R].trans.insert({ SIGN_LEFT_BRACKET, &ANY_to_nop });
        ptable[STATUS_R].trans.insert({ SIGN_RIGHT_BRACKET, &ANY_to_nop });
        ptable[STATUS_R].trans.insert({ SIGN_DOLLER, &ANY_to_nop });
        ptable[STATUS_R].trans.insert({ SIGN_DOT, &ANY_to_nop });
        ptable[STATUS_R].trans.insert({ SIGN_LEFT_SQUBRACE, &ANY_to_nop });

        ptable[STATUS_F].alpha_trans = &F_to_f0alpha;
        ptable[STATUS_F].trans.insert({ SIGN_LEFT_BRACKET, &F_to_s0lfbrack_E_s0rtbrack });
        ptable[STATUS_F].trans.insert({ SIGN_DOT, &F_to_f0anyalpha });
        ptable[STATUS_F].trans.insert({ SIGN_LEFT_SQUBRACE, &F_to_f0range });

        return ptable;
    }

    static constexpr Status_t STATUS_E = 0;
    static constexpr Status_t STATUS_En = 1;
    static constexpr Status_t STATUS_T = 2;
    static constexpr Status_t STATUS_Tn = 3;
    static constexpr Status_t STATUS_T1 = 4;
    static constexpr Status_t STATUS_R = 5;
    static constexpr Status_t STATUS_F = 6;
    static constexpr Status_t STATUS_NUM = 7;

    static constexpr Status_t ACTION_UNION = action_index_to_status(0);
    static constexpr Status_t ACTION_OR = action_index_to_status(1);
    static constexpr Status_t ACTION_REP = action_index_to_status(2);
    static constexpr Status_t ACTION_ALPHA = action_index_to_status(3);
    static constexpr Status_t ACTION_ONE_OR = action_index_to_status(4);
    static constexpr Status_t ACTION_ZERO_ONE = action_index_to_status(5);
    static constexpr Status_t ACTION_ANY_ALPHA = action_index_to_status(6);
    static constexpr Status_t ACTION_REP_FOR = action_index_to_status(7);
    static constexpr Status_t ACTION_RANGE = action_index_to_status(8);

    static constexpr UInt LEXER_BUFF_SIZE = Regex_lexer<Char_t, std::istream>::BUFF_SIZE;
    static Char_t lexer_buff_memory[LEXER_BUFF_SIZE];
    static const Vector<Status_t> Production_FAILURE;
    static Vector<Regex_LL1_trans> predicion_table;

    Vector<NFA_node<Char_t>> nfa;
    Status_t start_status;
    Small_vector_as_vec<Status_t> accept_state;
};
template <typename Char_t>
Char_t Basic_regex<Char_t>::lexer_buff_memory[LEXER_BUFF_SIZE];
template <typename Char_t>
const Vector<Status_t> Basic_regex<Char_t>::Production_FAILURE{};
template <>
Vector<Basic_regex<Char>::Regex_LL1_trans> Basic_regex<Char>::predicion_table{ init_predicion_table() };

using Regex = Basic_regex<Char>;

/**
 * @brief match the string with Basic_regex<Char_t>
 *
 * @tparam Char_t         the char type of the string to be matched, only support the type char for now
 * @tparam Identi_action  the function type of the function that will be called when match success or fail
 * @tparam Return_type    the return type of the function whose type is Identi_action
 */
template <typename Char_t, typename Identi_action, typename Return_type>
class Basic_regex_match
{
    static_assert(is_same_v<Char_t, Char>, "Basic_regex_match only support the type Char");

    template <typename Iter>
    friend std::pair<Iter, bool> regex_match(Regex& regex_nfa, Iter beg, Iter end);

    template <typename Iter>
    friend std::pair<Iter, size_t> regex_search(Regex& regex_nfa, Iter beg, Iter end);

public:
    using action_t = std::function<Identi_action>;

    Basic_regex_match() = default;

    Basic_regex_match(const Basic_regex_match&) = default;

    Basic_regex_match(Basic_regex_match&&) = default;

    Basic_regex_match(action_t success_act, action_t fail_act) : identify_actions{ fail_act, success_act } {}

    Basic_regex_match& operator=(const Basic_regex_match&) = default;

    Basic_regex_match& operator=(Basic_regex_match&&) = default;

    ~Basic_regex_match() = default;

    void reset_actions(action_t success_act, action_t fail_act)
    {
        identify_actions[0] = std::move(fail_act);
        identify_actions[1] = std::move(success_act);
    }

    template <typename Iter>
    std::pair<Return_type, bool> match_for(Basic_regex<Char_t>& regex_nfa, Iter beg, Iter end) const
    {
        Vector<Status_t> cur_status;
        Vector<Status_t> empty_closure;
        cur_status.reserve(10);
        empty_closure.reserve(10);
        Iter cursor = beg;
        size_t identify_nums = 0;
        cur_status.push_back(regex_nfa.start_status);
        collect_empty_closure(regex_nfa, cur_status, empty_closure);

        while (cursor != end) {
            next_status(regex_nfa, cur_status, empty_closure, cursor);
            if (cur_status.empty())
                return { identify_actions[0](cursor), false };
            empty_closure.clear();
            collect_empty_closure(regex_nfa, cur_status, empty_closure);
            ++cursor;
            ++identify_nums;
        }

        if (std::find(empty_closure.begin(), empty_closure.end(), regex_nfa.accept_state.back()) != empty_closure.end())
            return { identify_actions[1](cursor), true };
        else
            return { identify_actions[0](cursor), false };
    }

    template <typename Iter>
    std::pair<Return_type, size_t> search_for(Basic_regex<Char_t>& regex_nfa, Iter beg, Iter end) const
    {
        Vector<Status_t> cur_status;
        Hash_set<Status_t> empty_closure;
        cur_status.reserve(10);
        empty_closure.reserve(10);
        Iter cursor = beg;
        size_t identify_nums = 0;
        Iter last_accept_pos = beg;
        cur_status.push_back(regex_nfa.start_status);
        collect_empty_closure(regex_nfa, cur_status, empty_closure);

        while (cursor != end) {
            next_status(regex_nfa, cur_status, empty_closure, cursor);
            if (cur_status.empty())
                break;
            empty_closure.clear();
            collect_empty_closure(regex_nfa, cur_status, empty_closure);
            if (empty_closure.find(regex_nfa.accept_state.back()) != empty_closure.end())
                last_accept_pos = cursor + 1;
            ++cursor;
            ++identify_nums;
        }

        if (last_accept_pos != beg)
            return { identify_actions[1](last_accept_pos), identify_nums };
        else
            return { identify_actions[0](cursor), 0 };
    }

    template <typename Iter>
    static std::pair<Iter, bool> match(Basic_regex<Char_t>& regex_nfa, Iter beg, Iter end)
    {
        return simple_regex_match<Iter>().match_for(regex_nfa, beg, end);
    }

    template <typename Iter>
    static std::pair<Iter, size_t> search(Basic_regex<Char_t>& regex_nfa, Iter beg, Iter end)
    {
        return simple_regex_match<Iter>().search_for(regex_nfa, beg, end);
    }

private:
    template <typename Vec_or_HashSet>
    void next_status(Basic_regex<Char_t>& regex_nfa, Vector<Status_t>& cur_status, Vec_or_HashSet& empty_closure,
                     const Char_t* cursor) const
    {
        for (auto status : empty_closure) {
            auto& node = regex_nfa.nfa[status];
            auto result = node.trans_to(*cursor);
            if (result.first)
                cur_status.push_back(result.second);
        }
    }

    template <typename Vec_or_HashSet>
    void collect_empty_closure(Basic_regex<Char_t>& regex_nfa, Vector<Status_t>& source_set,
                               Vec_or_HashSet& result) const
    {
        Vector<bool> visited_node(regex_nfa.nfa.size());
        if constexpr (is_same_v<Vec_or_HashSet, Vector<Status_t>>) {
            result.insert(result.end(), source_set.begin(), source_set.end());
        } else {
            result.insert(source_set.begin(), source_set.end());
        }

        for (auto i : source_set)
            visited_node[i] = true;
        while (!source_set.empty()) {
            Status_t status = source_set.back();
            source_set.pop_back();
            auto trans_iter = regex_nfa.nfa.begin() + status;
            visited_node[status] = true;
            if (!trans_iter->has_empty_trans())
                continue;
            for (auto new_status : trans_iter->get_empty_trans()) {
                if (visited_node[new_status])
                    continue;
                source_set.push_back(new_status);
                if constexpr (is_same_v<Vec_or_HashSet, Vector<Status_t>>) {
                    result.push_back(new_status);
                } else {
                    result.insert(new_status);
                }
            }
        }
    }

    template <typename Iter>
    static Basic_regex_match<Char_t, Iter(Iter), Iter>& simple_regex_match()
    {
        static Basic_regex_match<Char_t, Iter(Iter), Iter> smatch([](Iter it) { return it; },
                                                                  [](Iter it) { return it; });
        return smatch;
    }

    Vector<std::function<Identi_action>> identify_actions;
};

template <typename Identi_action, typename Return_type>
using Regex_match = Basic_regex_match<Char, Identi_action, Return_type>;

template <typename Iter>
static std::pair<Iter, bool> regex_match(Regex& regex_nfa, Iter beg, Iter end)
{
    return Regex_match<void, void>::simple_regex_match<Iter>().match_for(regex_nfa, beg, end);
}

template <typename Iter>
static std::pair<Iter, size_t> regex_search(Regex& regex_nfa, Iter beg, Iter end)
{
    return Regex_match<void, void>::simple_regex_match<Iter>().search_for(regex_nfa, beg, end);
}
}  // namespace pcc

#endif  // REGEX_H_PCC_
