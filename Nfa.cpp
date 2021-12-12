//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Nfa.h"

#include <core/Formatting.h>

namespace sigil::nfa {

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

Arc *Automaton::create_epsilon_arc(State *origin, State *target)
{
    auto arc = m_arena.construct<Arc>(Arc::Type::Epsilon, origin, target);
    m_arcs.add(arc);
    return arc;
}

Arc *Automaton::create_character_arc(
    State *origin, State *target, CharSet char_set)
{
    auto arc = m_arena.construct<Arc>(Arc::Type::CharSet, origin, target);
    arc->char_set = std::move(char_set);
    m_arcs.add(arc);
    return arc;
}

inline static void log_state(core::StringBuilder &b, const State &state)
{
    if (state.accepting)
        Formatting::format_into(b, "(");
    if (state.start)
        Formatting::format_into(b, "*");
    Formatting::format_into(b, state.id);
    if (state.accepting)
        Formatting::format_into(b, ")");
}

inline static void format_indentation(core::StringBuilder &b, int level)
{
    for (auto i = 0; i < level; ++i) Formatting::format_into(b, "  ");
}

void Automaton::format(core::StringBuilder &b, const nfa::Automaton &automaton)
{
    Formatting::format_into(b, "nfa::Automaton {\n");
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
            if (arc->is_epsilon()) {
                if (arc->char_set.non_empty())
                    core::Formatting::format_into(b, ", ");

                core::Formatting::format_into(b, "epsilon");
            }

            Formatting::format_into(b, " ---> ");
            log_state(b, *arc->target);
            Formatting::format_into(b, "\n");
        }
    }
    Formatting::format_into(b, "}");
}

}  // namespace sigil::nfa
