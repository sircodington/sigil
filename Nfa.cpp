//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Nfa.h"

#include <core/Formatting.h>

namespace sigil::nfa {

template<typename T>
inline static void delete_all(List<T *> &list)
{
    for (auto ele : list) {
        if (ele == nullptr)
            continue;
        delete ele;
    }
    list.clear();
}

Automaton::~Automaton()
{
    delete_all(m_states);
    delete_all(m_arcs);
}

State *Automaton::create_state()
{
    auto state = new State { m_states.size() };
    m_states.add(state);
    return state;
}

Arc *Automaton::create_arc(const State *origin, const State *target)
{
    auto arc = new Arc { origin, target };
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
    Formatting:
        Formatting::format_into(b, "\n");

        for (const auto arc : automaton.arcs()) {
            if (arc->origin != state)
                continue;

            format_indentation(b, 2);
            Formatting::format_into(b, "--- ");

            core::Formatting::format_into(b, arc->char_set);
            if (arc->epsilon) {
                core::Formatting::format_into(b, ", epsilon");
            }

            Formatting::format_into(b, " ---> ");
            log_state(b, *arc->target);
            Formatting::format_into(b, "\n");
        }
    }
    Formatting::format_into(b, "}");
}

}  // namespace sigil::nfa
