//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/DfaTableScannerDriver.h>

namespace sigil {

DfaTableScannerDriver::DfaTableScannerDriver(
    List<State> transitions,
    List<TokenType> accepting,
    StaticTableScannerDriver underlying)
    : m_transitions(std::move(transitions))
    , m_accepting(std::move(accepting))
    , m_underlying(std::move(underlying))
{
}

DfaTableScannerDriver DfaTableScannerDriver::create(const dfa::Automaton &dfa)
{
    // @TODO: Minimize dfa (mfa) to get smaller tables
    State start_state = dfa.start_state()->id;
    State error_state = dfa.error_state()->id;
    const auto state_count = dfa.states().size();
    constexpr auto char_count = std::numeric_limits<u8>::max() + 1;
    const auto transition_count = state_count * char_count;
    List<State> transitions(transition_count);
    List<TokenType> accepting(state_count);

    for (Index i = 0; i < transition_count; ++i) transitions.add(error_state);
    for (Index i = 0; i < state_count; ++i)
        accepting.add(s32(SpecialTokenType::Error));

    for (const auto arc : dfa.arcs()) {
        // @FIXME: This loop is repeated several times
        for (auto c = sigil::CharSet::first; c <= sigil::CharSet::last; ++c) {
            if (not arc->char_set.contains(c))
                continue;

            const auto origin = State(arc->origin->id);
            const auto target = State(arc->target->id);
            transitions[table_index(origin, c)] = target;
        }
    }
    for (const auto state : dfa.states()) {
        if (state->is_accepting())
            accepting[state->id] = state->token_type;
    }

    StaticTable static_table(
        start_state, error_state, transitions.to_view(), accepting.to_view());
    StaticTableScannerDriver underlying(static_table);
    return DfaTableScannerDriver(
        std::move(transitions), std::move(accepting), std::move(underlying));
}

}  // namespace sigil
