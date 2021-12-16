//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <sigil/Dfa.h>
#include <sigil/ScannerDriver.h>

namespace sigil {

class DfaScannerDriver final : public ScannerDriver
{
public:
    explicit DfaScannerDriver(const dfa::Automaton &dfa);

private:
    [[nodiscard]] State start_state() const final;
    [[nodiscard]] State error_state() const final;
    [[nodiscard]] State next_state(State state, u8 c) const final;
    [[nodiscard]] bool is_accepting_state(State state) const final;
    [[nodiscard]] bool is_error_state(State state) const final;
    [[nodiscard]] TokenType accepting_token(State state) const final;

    [[nodiscard]] const dfa::State *state_by_id(s64) const;
    const dfa::Automaton &m_dfa;
};

}  // namespace sigil
