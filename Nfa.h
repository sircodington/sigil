//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/List.h>
#include <core/Logger.h>
#include <sigil/CharSet.h>

namespace sigil::nfa {

struct State
{
    const u64 id { 0 };
    bool start { false };
    bool accepting { false };
};

struct Arc
{
    const State *const origin { nullptr };
    const State *const target { nullptr };
    bool epsilon { false };
    CharSet char_set;
};

class Automaton
{
public:
    Automaton() = default;
    ~Automaton();

    State *create_state();
    Arc *create_arc(const State *origin, const State *target);

    [[nodiscard]] constexpr const List<State *> &states() const
    {
        return m_states;
    }
    [[nodiscard]] constexpr const List<Arc *> &arcs() const { return m_arcs; }

    static void log(const Automaton &);

private:
    List<State *> m_states;
    List<Arc *> m_arcs;
};

}  // namespace sigil::nfa
