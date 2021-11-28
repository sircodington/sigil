//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Nfa.h"

#include <core/Logging.h>

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

inline static void log_state(const State &state)
{
    if (state.accepting)
        Logging::log("(");
    if (state.start)
        Logging::log("*");
    Logging::log(state.id);
    if (state.accepting)
        Logging::log(")");
}

inline static void log_indentation(int level)
{
    for (auto i = 0; i < level; ++i) Logging::log("  ");
}

void Automaton::log(const nfa::Automaton &automaton)
{
    Logging::log("nfa::Automaton {\n");
    for (const auto state : automaton.states()) {
        log_indentation(1);
        log_state(*state);
        Logging::log("\n");

        for (const auto arc : automaton.arcs()) {
            if (arc->origin != state)
                continue;

            log_indentation(2);
            Logging::log("--- ");

            Logger<const CharSet &>::log(arc->char_set);
            if (arc->epsilon) {
                core::Logging::log(", epsilon");
            }

            Logging::log(" ---> ");
            log_state(*arc->target);
            Logging::log("\n");
        }
    }
    Logging::log("}");
}

}  // namespace sigil::nfa
