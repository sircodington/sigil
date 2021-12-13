//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Arena.h>
#include <core/Either.h>
#include <core/List.h>
#include <core/StringView.h>
#include <sigil/Dfa.h>
#include <sigil/Specification.h>

namespace sigil {

class Grammar
{
public:
    Grammar();
    ~Grammar() = default;
    Grammar(const Grammar &) = delete;
    Grammar(Grammar &&) = default;
    Grammar &operator=(const Grammar &) = delete;
    Grammar &operator=(Grammar &&) = default;

    static Either<StringView, Grammar> compile(const Specification &);

    core::Arena &arena() { return m_arena; }
    [[nodiscard]] const List<StringView> &token_names() const
    {
        return m_token_names;
    }
    List<StringView> &token_names() { return m_token_names; }
    [[nodiscard]] const dfa::Automaton &dfa() const { return m_dfa; }
    dfa::Automaton &dfa() { return m_dfa; }

private:
    core::Arena m_arena;
    List<StringView> m_token_names;
    dfa::Automaton m_dfa;
};

}  // namespace sigil
