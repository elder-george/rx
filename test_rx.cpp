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