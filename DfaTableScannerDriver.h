//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <sigil/Dfa.h>
#include <sigil/ScannerDriver.h>
#include <sigil/StaticTableScannerDriver.h>

namespace sigil {

class DfaTableScannerDriver final : public ScannerDriver
{
public:
    DfaTableScannerDriver(const DfaTableScannerDriver &) = delete;
    DfaTableScannerDriver(DfaTableScannerDriver &&) = default;
    DfaTableScannerDriver &operator=(const DfaTableScannerDriver &) = delete;
    DfaTableScannerDriver &operator=(DfaTableScannerDriver &&) = default;

    // @TODO: Option<DfaTableScannerDriver> for error handling
    static DfaTableScannerDriver create(const dfa::Automaton &);

    [[nodiscard]] StaticTable static_table() const
    {
        return m_underlying.static_table();
    }

    [[nodiscard]] State start_state() const final
    {
        return m_underlying.start_state();
    }
    [[nodiscard]] State error_state() const final
    {
        return m_underlying.error_state();
    }
    [[nodiscard]] State next_state(State state, u8 c) const final
    {
        return m_underlying.next_state(state, c);
    }
    [[nodiscard]] bool is_accepting_state(State state) const final
    {
        return m_underlying.is_accepting_state(state);
    }
    [[nodiscard]] bool is_error_state(State state) const final
    {
        return m_underlying.is_error_state(state);
    }
    [[nodiscard]] TokenType accepting_token(State state) const final
    {
        return m_underlying.accepting_token(state);
    }

private:
    DfaTableScannerDriver(
        List<State> transitions,
        List<TokenType> accepting,
        StaticTableScannerDriver underlying);

    inline static Index table_index(State state, u8 c)
    {
        constexpr auto char_count = std::numeric_limits<u8>::max() + 1;
        return c + state * char_count;
    }

    List<State> m_transitions;
    List<TokenType> m_accepting;
    StaticTableScannerDriver m_underlying;
};

}  // namespace sigil
