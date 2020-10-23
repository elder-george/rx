#pragma once

#include <vector>
#include <variant>

namespace Rx
{
    enum class Quant
    {
        One,
        ZeroOrOne,
        ZeroOrMore,
    };

    struct State
    {
        static State Wildcard(Quant q = Quant::One)
        {
            return {q, false};
        }

        static State Element(char c, Quant q = Quant::One)
        {
            return {q, c};
        }

        static State Group(std::vector<State> &&items, Quant q = Quant::One)
        {
            return {q, items};
        }

        Quant quant;
        std::variant<bool, char, std::vector<State>> details;
    };

    bool operator==(const State &a, const State &b);

    struct CompiledRe
    {
        static CompiledRe parse(const std::string_view &re);
        std::vector<State> states;
    };

} // namespace Rx