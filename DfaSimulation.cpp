//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "DfaSimulation.h"

#include <core/Formatting.h>

namespace sigil::dfa {

SimulationResult::SimulationResult(
    SimulationResult::Type type, StringView token_name)
    : m_type(type)
    , m_token_name(token_name)
{
}

SimulationResult SimulationResult::any() { return { Type::Any, {} }; }

SimulationResult SimulationResult::error() { return { Type::Error, {} }; }

SimulationResult SimulationResult::accept(StringView token_name)
{
    return { Type::Accept, token_name };
}

const dfa::State *transition(
    const dfa::Automaton &dfa, const dfa::State *state, char c)
{
    assert(state);

    const dfa::State *next = nullptr;

    for (const auto arc : dfa.arcs()) {
        if (arc->origin == state and arc->char_set.contains(c)) {
            assert(next == nullptr and "Transition is non-ambiguous");
            next = arc->target;
        }
    }
    assert(next);

    return next;
}

SimulationResult simulate(const sigil::Grammar &grammar, StringView source)
{
    const auto &dfa = grammar.dfa();

    auto state = dfa.start_state();

    for (Index i = 0; i < source.size(); ++i) {
        const auto c = source[i];
        state = transition(dfa, state, c);
    }

    if (state->is_error())
        return SimulationResult::error();
    if (state->is_accepting()) {
        const auto &token_name = grammar.token_names()[state->token_index];
        return SimulationResult::accept(token_name);
    }
    return SimulationResult::any();
}

}  // namespace sigil::dfa

namespace core {

void Formatter<sigil::dfa::SimulationResult>::format(
    StringBuilder &b, const sigil::dfa::SimulationResult &result)
{
    using Type = sigil::dfa::SimulationResult::Type;
    switch (result.type()) {
        case Type::Any:
            Formatting::format_into(b, "SimulationResult::Any");
            return;
        case Type::Error:
            Formatting::format_into(b, "SimulationResult::Error");
            return;
        case Type::Accept:
            Formatting::format_into(
                b, "SimulationResult::Accept(", result.token_name(), ")");
            return;
    }

    assert(false and "Unreachable");
}

}  // namespace core
