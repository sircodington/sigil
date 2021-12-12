//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Arena.h>
#include <core/Formatter.h>
#include <core/List.h>
#include <sigil/CharSet.h>

namespace sigil::nfa {

struct State
{
    explicit State(u64 id)
        : id(id)
    {
    }
    const u64 id { 0 };
    bool start { false };
    bool accepting { false };
};

struct Arc
{
    Arc(State *origin, State *target)
        : origin(origin)
        , target(target)
    {
    }
    State *origin { nullptr };
    State *target { nullptr };
    bool epsilon { false };
    CharSet char_set;
};

class Automaton
{
public:
    explicit Automaton(core::Arena &arena);
    ~Automaton();

    State *create_state();
    Arc *create_arc(State *origin, State *target);

    [[nodiscard]] constexpr const List<State *> &states() const
    {
        return m_states;
    }
    [[nodiscard]] constexpr const List<Arc *> &arcs() const { return m_arcs; }

    static void format(core::StringBuilder &, const Automaton &);

private:
    core::Arena &m_arena;
    List<State *> m_states;
    List<Arc *> m_arcs;
};

}  // namespace sigil::nfa
