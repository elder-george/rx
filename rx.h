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
        static State Wildcard()
        {
            return {Quant::One,  false};
        }

        static State Element(char c)
        {
            return {Quant::One, c};
        }

        static State Group(std::vector<State>&& items)
        {
            return {Quant::One,items };
        }

        Quant quant;
        std::variant<bool, char, std::vector<State>> details;
    };

    bool operator==(const State& a, const State& b);

    struct CompiledRe
    {
        static CompiledRe parse(const std::string_view &re);
        std::vector<State> states;
    };

} // namespace Rx