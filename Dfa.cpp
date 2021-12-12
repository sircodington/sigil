//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Dfa.h"

#include <core/Formatting.h>

namespace sigil::dfa {

Automaton::Automaton(core::Arena &arena)
    : m_arena(arena)
{
}

Automaton::~Automaton()
{
    m_states.clear();
    m_arcs.clear();
}

State *Automaton::create_state()
{
    auto state = m_arena.construct<State>(m_states.size());
    m_states.add(state);
    return state;
}

Arc *Automaton::create_arc(State *origin, State *target, CharSet char_set)
{
    auto arc = m_arena.construct<Arc>(origin, target);
    arc->char_set = std::move(char_set);
    m_arcs.add(arc);
    return arc;
}

}  // namespace sigil::dfa

namespace core {

inline static void log_state(
    core::StringBuilder &b, const sigil::dfa::State &state)
{
    if (state.is_accepting())
        Formatting::format_into(b, "(");
    if (state.start)
        Formatting::format_into(b, "*");
    Formatting::format_into(b, state.id);
    if (state.is_error())
        Formatting::format_into(b, "!");
    if (state.is_accepting()) {
        Formatting::format_into(b, ",");
        Formatting::format_into(b, state.token_index);
        Formatting::format_into(b, ")");
    }
}

inline static void format_indentation(core::StringBuilder &b, int level)
{
    for (auto i = 0; i < level; ++i) Formatting::format_into(b, "  ");
}

void Formatter<sigil::dfa::Automaton>::format(
    StringBuilder &b, const sigil::dfa::Automaton &automaton)
{
    Formatting::format_into(b, "dfa::Automaton {\n");
    for (const auto state : automaton.states()) {
        format_indentation(b, 1);
        log_state(b, *state);
        Formatting::format_into(b, "\n");

        for (const auto arc : automaton.arcs()) {
            if (arc->origin != state)
                continue;

            format_indentation(b, 2);
            Formatting::format_into(b, "--- ");
            core::Formatting::format_into(b, arc->char_set);
            Formatting::format_into(b, " ---> ");
            log_state(b, *arc->target);
            Formatting::format_into(b, "\n");
        }
    }
    Formatting::format_into(b, "}");
}

}  // namespace core
