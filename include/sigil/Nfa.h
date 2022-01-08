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
    enum class Type : u8
    {
        Epsilon,
        CharSet,
    };
    Arc(Type type, State *origin, State *target)
        : type(type)
        , origin(origin)
        , target(target)
    {
    }

    [[nodiscard]] bool is_epsilon() const { return Type::Epsilon == type; }
    [[nodiscard]] bool is_character() const { return Type::CharSet == type; }

    Type type;
    State *origin { nullptr };
    State *target { nullptr };
    CharSet char_set;
};

class Automaton
{
public:
    explicit Automaton(core::Arena &arena);
    Automaton(const Automaton &) = delete;
    Automaton(Automaton &&) = default;
    ~Automaton();

    Automaton &operator=(const Automaton &) = delete;
    Automaton &operator=(Automaton &&) = default;

    State *create_state();
    Arc *create_epsilon_arc(State *origin, State *target);
    Arc *create_character_arc(
        State *origin, State *target, CharSet char_set = CharSet());

    [[nodiscard]] constexpr const List<State *> &states() const
    {
        return m_states;
    }
    [[nodiscard]] constexpr const List<Arc *> &arcs() const { return m_arcs; }

    [[nodiscard]] nfa::State *start_state() const;

private:
    core::Arena &arena() { return *m_arena; }

    core::Arena *m_arena { nullptr };
    List<State *> m_states;
    List<Arc *> m_arcs;
};

}  // namespace sigil::nfa

namespace core {

template<>
class Formatter<sigil::nfa::Automaton>
{
public:
    static void format(StringBuilder &, const sigil::nfa::Automaton &);
};

}  // namespace core
