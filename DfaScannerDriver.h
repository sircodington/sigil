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
    u32 start_state() const final;
    u32 error_state() const final;
    s64 next_state(s64 state, s32 character) const final;
    s64 is_accepting_state(s64 state) const final;
    s64 is_error_state(s64 state) const final;
    s64 accepting_token(s64 state) const final;

private:
    const dfa::State *state_by_id(s64) const;
    const dfa::Automaton &m_dfa;
};

}  // namespace sigil
