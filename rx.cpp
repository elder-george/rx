#include "rx.h"

#include <stdexcept>

namespace Rx
{

    bool operator==(const State& a, const State& b){
        return a.quant == b.quant && a.details == b.details;
    }
    CompiledRe CompiledRe::parse(const std::string_view &re)
    {
        std::vector<std::vector<State>> stack;
        stack.reserve(re.size());
        stack.push_back({});
        for (auto i = 0; i < re.size(); ++i)
        {
            switch (re[i])
            {
            case '.':
                stack.back().push_back(State::Wildcard());
                break;
            case '\\':
                if (i + 1 > re.size())
                {
                    throw std::logic_error("bad escape character");
                }
                stack.back().push_back(State::Element(re[i + 1]));
                ++i;
                break;
            case '(':
                stack.push_back({});
                break;
            case ')':
            {
                if (stack.size() <= 1)
                {
                    throw std::logic_error("No group to close");
                }
                auto states = stack.back();
                stack.pop_back();
                stack.back().push_back(State::Group(std::move(states)));
                break;
            }
            case '?':
            {
                auto &lastElem = stack.back().back();
                if (lastElem.quant != Quant::One)
                {
                    throw std::logic_error("Only one quantifier allowed");
                }
                lastElem.quant = Quant::ZeroOrOne;
                break;
            }
            case '*':
            {
                auto &lastElem = stack.back().back();
                if (lastElem.quant != Quant::One)
                {
                    throw std::logic_error("Only one quantifier allowed");
                }
                lastElem.quant = Quant::ZeroOrMore;
                break;
            }
            case '+':
            {
                auto &lastElem = stack.back().back();
                if (lastElem.quant != Quant::One)
                {
                    throw std::logic_error("Only one quantifier allowed");
                }
                auto zeroOrMoreCopy = lastElem;
                zeroOrMoreCopy.quant = Quant::ZeroOrMore;
                stack.back().push_back(zeroOrMoreCopy);
                break;
            }
            default:
                stack.back().push_back(State::Element(re[i]));
            }
        }

        if (stack.size() != 1)
        {
            throw std::logic_error("Unmatched groups");
        }
        return {stack[0]};
    }
} // namespace Rx