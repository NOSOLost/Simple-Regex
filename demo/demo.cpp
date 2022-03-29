#include "regex.h"
#include "test_tools.h"
using namespace pcc;
using namespace pcc_test;
using namespace std;

void see_result(const Char* regex_str, const Char* beg, const Char* end, bool use_match);

template <typename Match_fn, typename Search_fn>
void try_match_search(const Char* regex_str, const Char* pattern1, const Char* pattern2, Match_fn match_fn,
                      Search_fn search_fn);

int main()
{
    using Match_type = Regex_match<const Char*(const Char*), const Char*>;

    // (1) use regex_match(), regex_search()
    const Char* regex_str = "(ab[e-h]){3,3}";
    const Char* pattern1 = "abeabfabh";
    const Char* pattern2 = "abeabfabhRabe";
    try_match_search(
        regex_str, pattern1, pattern2,
        [&](Regex& regex) { return regex_match(regex, pattern1, pattern1 + strlen(pattern1)); },
        [&](Regex& regex) { return regex_search(regex, pattern2, pattern2 + strlen(pattern2)); });

    // (2) use Regex_match::match Regex_match::search
    regex_str = "[^a-zA-Z0-9]*([x-zep]|RE)+";
    pattern1 = "$&^#xxyzyyeREREREepyyp";
    pattern2 = "$&^#xxyzyyepREREREepyypARE";
    try_match_search(
        regex_str, pattern1, pattern2,
        [&](Regex& regex) { return Match_type::match(regex, pattern1, pattern1 + strlen(pattern1)); },
        [&](Regex& regex) { return Match_type::search(regex, pattern2, pattern2 + strlen(pattern2)); });

    // (3) use Regex_match.match_for(), Regex_match.search_for()
    regex_str = "$(sr|(ab*c+|[f-h]+|(rep)*){2,5}|s*)${3,6}";
    pattern1 = "$abbbbbcccreprepfghgrepreph$$$$";
    pattern2 = "$abbbbbcccreprepfghgrepreph$$$$$$$$$$";
    Match_type rgmatch([](const Char* c) { return c; }, [](const Char* c) { return nullptr; });
    try_match_search(
        regex_str, pattern1, pattern2,
        [&](Regex& regex) { return rgmatch.match_for(regex, pattern1, pattern1 + strlen(pattern1)); },
        [&](Regex& regex) { return rgmatch.search_for(regex, pattern2, pattern2 + strlen(pattern2)); });

    // (4) use Regex_match.match_for(), Regex_match.search_for() fail
    regex_str = "$(sr|(ab*c+|[f-h]+|(rep)*){2,5}|s*)${3,6}";
    pattern1 = "$abbbbbcccreprepfghgrepreph$$";
    pattern2 = "$abbbbbcccreprepfghgrepreph$$";
    try_match_search(
        regex_str, pattern1, pattern2,
        [&](Regex& regex) { return rgmatch.match_for(regex, pattern1, pattern1 + strlen(pattern1)); },
        [&](Regex& regex) { return rgmatch.search_for(regex, pattern2, pattern2 + strlen(pattern2)); });

    // (5) wrong regex
    regex_str = "(ab|(c+d|[e-h]+z)e";
    try_match_search(
        regex_str, pattern1, pattern2,
        [&](Regex& regex) {
            exit(1);
            return std::make_pair("", false);
        },
        [&](Regex& regex) {
            exit(1);
            return std::make_pair("", size_t(0));
        });

    println("Success");
}

void see_result(const Char* regex_str, const Char* beg, const Char* end, bool use_match)
{
    if (beg != end) {
        print("<", regex_str, "> ", use_match ? "match" : "search", " <", beg, "> get result : <");
        for_each(beg, end, [](Char c) { std::cout << c; });
        println(">");
    } else {
        println("<", regex_str, "> ", use_match ? " match" : "search", " <", beg, "> FAIL");
    }
}

template <typename Match_fn, typename Search_fn>
void try_match_search(const Char* regex_str, const Char* pattern1, const Char* pattern2, Match_fn match_fn,
                      Search_fn search_fn)
{
    Regex regex;
    if (!regex.regenetare_regex(regex_str)) {
        println("generate regex <", regex_str, "> FAIL\n");
        return;
    }

    std::pair<const Char*, bool> result = match_fn(regex);
    auto end = (result.second ? result.first : pattern1);
    see_result(regex_str, pattern1, end, true);

    std::pair<const Char*, size_t> result1 = search_fn(regex);
    auto end2 = (result1.second != 0 ? result1.first : pattern2);
    see_result(regex_str, pattern2, end2, false);
    println();
}