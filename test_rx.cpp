#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "rx.h"

#include <iostream>

using std::make_shared;
using std::shared_ptr;
using std::vector;

using namespace Rx;

namespace std
{
    // Pretty-printing
    ostream &operator<<(ostream &os, const State &st);
    ostream &operator<<(ostream &os, Quant q)
    {
        return (os << (q == Quant::One
                           ? "One"
                           : q == Quant::ZeroOrMore
                                 ? "ZeroOrMore"
                                 : "ZeroOrOne"));
    }
    ostream &operator<<(ostream &os, const std::vector<State> &items)
    {
        os << "[ ";
        for (auto &it : items)
        {
            os << it << " ";
        }
        os << "]";
        return os;
    }
    ostream &operator<<(ostream &os, const State &st)
    {
        if (auto ch = std::get_if<char>(&st.details))
        {
            os << "Element{ '" << *ch << "', " << st.quant << " }";
        }
        else if (auto group = std::get_if<std::vector<State>>(&st.details))
        {
            os << "Group{ " << *group << ", " << st.quant << " }";
        }
        else
        {
            os << "Wildcard{ " << st.quant << " }";
        }
        return os;
    }

    template <typename T>
    ostream &operator<<(ostream &os, const std::optional<T> &opt)
    {
        if (opt)
        {
            os << *opt;
        }
        else
        {
            os << "none";
        }
        return os;
    }
} // namespace std

TEST_CASE("parse single char", "[parse]")
{
    auto result = parse("a");
    REQUIRE(result == vector<State>{State::Element('a')});
}

TEST_CASE("parse escaping", "[parse]")
{
    auto result = parse("\\*\\?\\+");
    REQUIRE(result == vector<State>{State::Element('*'), State::Element('?'), State::Element('+')});
}

TEST_CASE("parse sequence", "[parse]")
{
    auto result = parse("abc");
    REQUIRE(result == vector<State>{State::Element('a'), State::Element('b'), State::Element('c')});
}

TEST_CASE("parse wildcard", "[parse]")
{
    auto result = parse(".");
    REQUIRE(result == vector<State>{State::Wildcard()});
}

TEST_CASE("parse zero or one", "[parse]")
{
    auto result = parse("ab?c");
    REQUIRE(result == vector<State>{State::Element('a'), State::Element('b', Quant::ZeroOrOne), State::Element('c')});
}

TEST_CASE("parse zero or more", "[parse]")
{
    auto result = parse("ab*c");
    REQUIRE(result == vector<State>{State::Element('a'), State::Element('b', Quant::ZeroOrMore), State::Element('c')});
}

TEST_CASE("parse one or more", "[parse]")
{
    auto result = parse("ab+c");
    REQUIRE(result == vector<State>{State::Element('a'), State::Element('b'), State::Element('b', Quant::ZeroOrMore), State::Element('c')});
}

TEST_CASE("parse group", "[parse]")
{
    auto result = parse("(abc)");
    REQUIRE(result == vector<State>{
                          State::Group(vector<State>{
                              State::Element('a'),
                              State::Element('b'),
                              State::Element('c')})});
}

TEST_CASE("match empty string", "[match]")
{
    REQUIRE(match(parse(""), "abc") == std::optional{0});
    REQUIRE(match(parse(""), "") == std::optional{0});
}

TEST_CASE("match single char", "[match]")
{
    REQUIRE(match(parse("a"), "abc") == std::optional{1});
    REQUIRE(match(parse("a"), "bc") == std::nullopt);
    REQUIRE(match(parse("a"), "") == std::nullopt);
}

TEST_CASE("match wildcard", "[match]")
{
    REQUIRE(match(parse(".."), "abc") == std::optional{2});
    REQUIRE(match(parse(".."), "") == std::nullopt);
}

TEST_CASE("match sequence", "[match]")
{
    REQUIRE(match(parse("abc"), "abc") == std::optional{3});
    REQUIRE(match(parse("abc"), "bca") == std::nullopt);
}

TEST_CASE("match *", "[match]")
{
    REQUIRE(match(parse("ab*c"), "abbbbbc") == std::optional{7});
    REQUIRE(match(parse("ab*c"), "ac") == std::optional{2});
}

TEST_CASE("match +", "[match]")
{
    REQUIRE(match(parse("ab+c"), "abbbbbc") == std::optional{7});
    REQUIRE(match(parse("ab+c"), "ac") == std::nullopt);
}

TEST_CASE("match groups", "[match]")
{
    REQUIRE(match(parse("(abc)"), "abc") == std::optional{3});
    REQUIRE(match(parse("(abc)?"), "abc") == std::optional{3});
    REQUIRE(match(parse("(abc)?"), "") == std::optional{0});
    REQUIRE(match(parse("(abc)*"), "abcabc") == std::optional{6});
}

// TEST_CASE("has backtacking", "[match]")
// {
//     REQUIRE(match(parse("ac*c"), "acc") == std::optional{7});
//     REQUIRE(match(parse("ac*c"), "ac") == std::optional{2});
// }