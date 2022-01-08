//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <sigil/Dfa.h>
#include <sigil/ScannerDriver.h>
#include <sigil/StaticTable.h>

namespace sigil {

class StaticTableScannerDriver final : public ScannerDriver
{
public:
    StaticTableScannerDriver(const StaticTableScannerDriver &) = delete;
    StaticTableScannerDriver(StaticTableScannerDriver &&) = default;
    StaticTableScannerDriver &operator=(const StaticTableScannerDriver &) =
        delete;
    StaticTableScannerDriver &operator=(StaticTableScannerDriver &&) = default;

    explicit StaticTableScannerDriver(const StaticTable &);

    [[nodiscard]] StaticTable static_table() const;

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

private:
    inline static Index table_index(State state, u8 c)
    {
        constexpr auto char_count = std::numeric_limits<u8>::max() + 1;
        return c + state * char_count;
    }

    State m_start_state;
    State m_error_state;
    ListView<State> m_transitions;
    ListView<TokenType> m_accepting;
};

}  // namespace sigil
