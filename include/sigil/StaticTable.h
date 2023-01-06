//
// Copyright (c) 2021-2023, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Formatter.h>
#include <core/ListView.h>

#include <sigil/Array.h>
#include <sigil/Types.h>

namespace sigil {

// @TODO: Compress table, it looks pretty sparse to me
class StaticTable
{
public:
    StaticTable(
        State start_state,
        State error_state,
        Array<State> transitions,
        Array<TokenType> accepting);

    [[nodiscard]] State start_state() const { return m_start_state; }
    [[nodiscard]] State error_state() const { return m_error_state; }
    [[nodiscard]] Array<State> transitions() const { return m_transitions; }
    [[nodiscard]] Array<TokenType> accepting() const { return m_accepting; }

private:
    State m_start_state;
    State m_error_state;
    Array<State> m_transitions;
    Array<TokenType> m_accepting;
};

}  // namespace sigil

namespace core {

template<>
class Formatter<sigil::StaticTable>
{
public:
    static void format(StringBuilder &, const sigil::StaticTable &);
};

}  // namespace core
