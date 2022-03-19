//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/DfaScannerDriver.h>

namespace sigil {

DfaScannerDriver::DfaScannerDriver(const dfa::Automaton &dfa)
    : m_dfa(dfa)
{
}

State DfaScannerDriver::start_state() const
{
    auto state = m_dfa.start_state();
    assert(state);
    return State(state->id);
}

State DfaScannerDriver::error_state() const
{
    dfa::State *state = nullptr;
    for (auto st : m_dfa.states()) {
        if (st->is_error()) {
            assert(
                state == nullptr and
                "There should be only one explicit error state");
            state = st;
        }
    }

    assert(state);
    return State(state->id);
}

State DfaScannerDriver::next_state(State state, u8 c) const
{
    auto source = state_by_id(state);
    assert(source);

    for (const auto arc : m_dfa.arcs()) {
        if (arc->origin == source and arc->char_set.contains(c))
            return State(arc->target->id);
    }

    assert(false and "Unreachable");
    return error_state();
}

bool DfaScannerDriver::is_accepting_state(State state) const
{
    auto l_state = state_by_id(state);
    assert(l_state);
    return l_state->is_accepting();
}

bool DfaScannerDriver::is_error_state(State state) const
{
    auto l_state = state_by_id(state);
    assert(l_state);
    return l_state->is_error();
}

TokenType DfaScannerDriver::accepting_token(State state) const
{
    assert(is_accepting_state(state));
    return state_by_id(state)->token_type;
}

const dfa::State *DfaScannerDriver::state_by_id(State id) const
{
    if (id < 0)
        return nullptr;
    if (not m_dfa.states().in_bounds(id))
        return nullptr;
    return m_dfa.states()[id];
}

}  // namespace sigil