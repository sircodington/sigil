//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "DfaScannerDriver.h"

namespace sigil {

DfaScannerDriver::DfaScannerDriver(const dfa::Automaton &dfa)
    : m_dfa(dfa)
{
}

u32 DfaScannerDriver::start_state() const
{
    auto state = m_dfa.start_state();
    assert(state);
    return u32(state->id);
}

u32 DfaScannerDriver::error_state() const
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
    return u32(state->id);
}

s64 DfaScannerDriver::next_state(s64 state, s32 character) const
{
    auto source = state_by_id(state);
    assert(source);

    for (const auto arc : m_dfa.arcs()) {
        if (arc->origin == source and arc->char_set.contains(character))
            return s64(arc->target->id);
    }

    assert(false and "Unreachable");
    return -1;
}

s64 DfaScannerDriver::is_accepting_state(s64 state) const
{
    auto l_state = state_by_id(state);
    assert(l_state);
    return l_state->is_accepting();
}

s64 DfaScannerDriver::is_error_state(s64 state) const
{
    auto l_state = state_by_id(state);
    assert(l_state);
    return l_state->is_error();
}

s64 DfaScannerDriver::accepting_token(s64 state) const
{
    assert(is_accepting_state(state));
    return state_by_id(state)->token_index;
}

const dfa::State *DfaScannerDriver::state_by_id(s64 id) const
{
    if (id < 0)
        return nullptr;
    if (not m_dfa.states().in_bounds(id))
        return nullptr;
    return m_dfa.states()[id];
}

}  // namespace sigil