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

namespace sigil::dfa {

struct State
{
    enum class Type : u8
    {
        Invalid,
        Error,
        Accepting,
    };
    explicit State(u64 id)
        : id(id)
    {
    }

    [[nodiscard]] bool is_accepting() const { return Type::Accepting == type; }
    [[nodiscard]] bool is_error() const { return Type::Error == type; }

    const u64 id { 0 };
    bool start { false };
    Type type { Type::Invalid };
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
    CharSet char_set;
};

class Automaton
{
public:
    explicit Automaton(core::Arena &arena);
    ~Automaton();

    State *create_state();
    Arc *create_arc(State *origin, State *target, CharSet char_set = CharSet());

    [[nodiscard]] constexpr const List<State *> &states() const
    {
        return m_states;
    }
    [[nodiscard]] constexpr const List<Arc *> &arcs() const { return m_arcs; }

private:
    core::Arena &m_arena;
    List<State *> m_states;
    List<Arc *> m_arcs;
};

}  // namespace sigil::dfa

namespace core {

template<>
class Formatter<sigil::dfa::Automaton>
{
public:
    static void format(StringBuilder &, const sigil::dfa::Automaton &);
};

}  // namespace core