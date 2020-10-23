#pragma once

#include <vector>
#include <variant>
#include <optional>

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

    std::vector<State> parse(const std::string_view &re);

    std::optional<size_t> match(const std::vector<State>& states, const std::string_view &str);
} // namespace Rx