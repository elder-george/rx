#include "rx.h"

#include <stdexcept>
#include <algorithm>

namespace Rx
{

    bool operator==(const State &a, const State &b)
    {
        return a.quant == b.quant && a.details == b.details;
    }
    std::vector<State> parse(const std::string_view &re)
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

    struct BacktrackState
    {
        bool isBacktrackable;
        State state;
        std::vector<size_t> consumptions;
    };

    struct Matcher
    {
        Matcher(const std::vector<State> &states)
        {
            statesStack = states;
            std::reverse(statesStack.begin(), statesStack.end());
        }

        std::tuple<bool, size_t> match(const std::string_view &str)
        {
            while (!statesStack.empty())
            {
                currentState = std::move(statesStack.back());
                statesStack.pop_back();
                switch (currentState.quant)
                {
                case Quant::One:
                {
                    auto [isMatch, consumed] = matchesStringAt(currentState, str, i);
                    if (!isMatch)
                    {
                        auto indexBeforeBacktracking = i;
                        if (!backtrack())
                        {
                            return {false, indexBeforeBacktracking};
                        }
                        statesStack.push_back(currentState);
                        continue;
                    }
                    backtrackStack.push_back({false, currentState, {consumed}});
                    i += consumed;
                    continue;
                }
                case Quant::ZeroOrOne:
                {
                    if (i >= str.size())
                    {
                        backtrackStack.push_back({false, currentState, {0}});
                        continue;
                    }
                    auto [isMatch, consumed] = matchesStringAt(currentState, str, i);
                    i += consumed;
                    backtrackStack.push_back({isMatch && consumed > 0, currentState, {consumed}});
                    continue;
                }
                case Quant::ZeroOrMore:
                    BacktrackState btState{true, currentState, {}};
                    while (true)
                    {
                        if (i >= str.size())
                        {
                            if (btState.consumptions.size() == 0)
                            {
                                btState.isBacktrackable = false;
                                btState.consumptions.push_back(0);
                            }
                            backtrackStack.push_back(btState);
                            break;
                        }
                        auto [isMatch, consumed] = matchesStringAt(currentState, str, i);
                        if (!isMatch || consumed == 0)
                        {
                            if (btState.consumptions.size() == 0)
                            {
                                btState.isBacktrackable = false;
                                btState.consumptions.push_back(0);
                            }
                            backtrackStack.push_back(btState);
                            break;
                        }
                        btState.consumptions.push_back(consumed);
                        i += consumed;
                    }
                }
            }
            return {true, i};
        }

        bool backtrack()
        {
            statesStack.push_back(currentState);
            bool couldBacktrack = false;

            while (backtrackStack.size() > 0)
            {
                auto btState = backtrackStack.back();
                backtrackStack.pop_back();

                if (btState.isBacktrackable)
                {
                    if (btState.consumptions.size() == 0)
                    {
                        statesStack.push_back(btState.state);
                        continue;
                    }
                    else
                    {
                        auto n = btState.consumptions.back();
                        i -= n;
                        couldBacktrack = true;
                        break;
                    }
                }
                else
                {
                    statesStack.push_back(btState.state);
                    for (auto n : btState.consumptions)
                    {
                        i -= n;
                    }
                }
            }
            if (couldBacktrack)
            {
                currentState = statesStack.back();
                statesStack.pop_back();
            }
            return couldBacktrack;
        }

        static std::tuple<bool, size_t> matchesStringAt(const State &st, const std::string_view &str, size_t i)
        {
            if (i >= str.size())
            {
                return {false, 0};
            }

            if (auto c = std::get_if<char>(&st.details)) // Element
            {
                if (*c == str[i])
                {
                    return {true, 1};
                }
                else
                {
                    return {false, 0};
                }
            }
            else if (auto items = std::get_if<std::vector<State>>(&st.details)) // Group
            {
                return Matcher{*items}.match(str.substr(i));
            }
            else // Wildcard
            {
                return {true, 1}; // Wildcard always matches
            }
        }

        std::vector<State> statesStack{};
        State currentState{};
        size_t i{};
        std::vector<BacktrackState> backtrackStack{};
    };

    std::optional<size_t> match(const std::vector<State> &states, const std::string_view &str)
    {
        Matcher m{states};
        auto [isMatch, consumed] = m.match(str);
        if (isMatch)
            return std::optional(consumed);
        else
            return {};
    }
} // namespace Rx