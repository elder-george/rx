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
        } else {
            os << "Wildcard{ "<<st.quant<<" }";
        }
        return os;
    }

} // namespace std

bool operator==(const State &, const State &) { return true; }

TEST_CASE("single char")
{
    auto result = CompiledRe::parse("a");
    REQUIRE(result.states == vector<State>{State::Element('a')});
}