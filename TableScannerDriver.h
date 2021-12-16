//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <sigil/Dfa.h>
#include <sigil/ScannerDriver.h>

namespace sigil {

class TableScannerDriver final : public ScannerDriver
{
public:
    TableScannerDriver(const TableScannerDriver &) = delete;
    TableScannerDriver(TableScannerDriver &&) = default;
    TableScannerDriver &operator=(const TableScannerDriver &) = delete;
    TableScannerDriver &operator=(TableScannerDriver &&) = default;

    // @TODO: Option<TableScannerDriver> for error handling
    static TableScannerDriver create(const dfa::Automaton &);

private:
    TableScannerDriver(
        State start_state,
        State error_state,
        List<State> transitions,
        List<TokenType> accepting);

    inline static Index table_index(State state, u8 c)
    {
        constexpr auto char_count = std::numeric_limits<u8>::max() + 1;
        return c + state * char_count;
    }

    [[nodiscard]] State start_state() const final { return m_start_state; }
    [[nodiscard]] State error_state() const final { return m_error_state; }

    [[nodiscard]] State next_state(State state, u8 c) const final
    {
        return m_transitions[table_index(state, c)];
    }
    [[nodiscard]] bool is_accepting_state(State state) const final
    {
        return accepting_token(state) >= 0;
    }
    [[nodiscard]] bool is_error_state(State state) const final
    {
        return error_state() == state;
    }
    [[nodiscard]] TokenType accepting_token(State state) const final
    {
        return m_accepting[state];
    }

    State m_start_state;
    State m_error_state;
    List<State> m_transitions;
    List<TokenType> m_accepting;
};

}  // namespace sigil
