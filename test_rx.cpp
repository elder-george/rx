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

TEST_CASE("single char")
{
    auto result = CompiledRe::parse("a");
    REQUIRE(result.states == vector<State>{State::Element('a')});
}

TEST_CASE("escaping")
{
    auto result = CompiledRe::parse("\\*\\?\\+");
    REQUIRE(result.states == vector<State>{State::Element('*'), State::Element('?'), State::Element('+')});
}

TEST_CASE("sequence")
{
    auto result = CompiledRe::parse("abc");
    REQUIRE(result.states == vector<State>{State::Element('a'), State::Element('b'), State::Element('c')});
}

TEST_CASE("wildcard")
{
    auto result = CompiledRe::parse(".");
    REQUIRE(result.states == vector<State>{State::Wildcard()});
}

TEST_CASE("zero or one")
{
    auto result = CompiledRe::parse("ab?c");
    REQUIRE(result.states == vector<State>{State::Element('a'), State::Element('b', Quant::ZeroOrOne), State::Element('c')});
}

TEST_CASE("zero or more")
{
    auto result = CompiledRe::parse("ab*c");
    REQUIRE(result.states == vector<State>{State::Element('a'), State::Element('b', Quant::ZeroOrMore), State::Element('c')});
}

TEST_CASE("one or more")
{
    auto result = CompiledRe::parse("ab+c");
    REQUIRE(result.states == vector<State>{State::Element('a'), State::Element('b'), State::Element('b', Quant::ZeroOrMore), State::Element('c')});
}

TEST_CASE("group")
{
    auto result = CompiledRe::parse("(abc)");
    REQUIRE(result.states == vector<State>{
                                 State::Group(vector<State>{
                                     State::Element('a'),
                                     State::Element('b'),
                                     State::Element('c')})});
}