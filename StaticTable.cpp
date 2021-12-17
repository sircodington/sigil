//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "StaticTable.h"

#include <core/Formatting.h>

namespace sigil {

StaticTable::StaticTable(
    State start_state,
    State error_state,
    ListView<State> transitions,
    ListView<TokenType> accepting)
    : m_start_state(start_state)
    , m_error_state(error_state)
    , m_transitions(transitions)
    , m_accepting(accepting)
{
}

}  // namespace sigil

namespace core {

template<typename T>
inline static void format_array_literal(
    StringBuilder &b, const ListView<T> &view)
{
    Formatting::format_into(b, "{");

    auto first = true;
    for (const auto &elem : view) {
        if (not first)
            Formatting::format_into(b, ",");
        first = false;

        Formatting::format_into(b, elem);
    }

    Formatting::format_into(b, "}");
}

inline static void format_list_view(
    StringBuilder &b,
    const StringView &element_type,
    const StringView &data_member)
{
    Formatting::format_into(
        b,
        "core::ListView<",
        element_type,
        ">(",
        data_member,
        ",",
        "sizeof(",
        data_member,
        ") / sizeof(",
        data_member,
        "[0])",
        ")");
}

void Formatter<sigil::StaticTable>::format(
    StringBuilder &b, const sigil::StaticTable &table)
{
    Formatting::format_into(b, "({");

    const StringView transition_table_data_member("transitions");
    Formatting::format_into(
        b, "sigil::State ", transition_table_data_member, "[] = ");
    format_array_literal(b, table.transitions());
    Formatting::format_into(b, ";");

    const StringView accepting_table_data_member("accepting");
    Formatting::format_into(
        b, "sigil::TokenType ", accepting_table_data_member, "[] = ");
    format_array_literal(b, table.accepting());
    Formatting::format_into(b, ";");

    Formatting::format_into(
        b,
        "sigil::StaticTable(",
        table.start_state(),
        ",",
        table.error_state(),
        ",");
    format_list_view(b, "sigil::State", transition_table_data_member);
    Formatting::format_into(b, ",");
    format_list_view(b, "sigil::TokenType", accepting_table_data_member);
    Formatting::format_into(b, ");})");
}

}  // namespace core