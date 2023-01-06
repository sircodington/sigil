//
// Copyright (c) 2021-2023, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/StaticTable.h>

#include <core/Formatting.h>

namespace sigil {

StaticTable::StaticTable(
    State start_state,
    State error_state,
    Array<State> transitions,
    Array<TokenType> accepting)
    : m_start_state(start_state)
    , m_error_state(error_state)
    , m_transitions(transitions)
    , m_accepting(accepting)
{
}

}  // namespace sigil

namespace core {

template<typename T>
inline static void format_sigil_array(
    StringBuilder &b, core::StringView type, const sigil::Array<T> &array)
{
    Formatting::format_into(b, "sigil::Array<"sv, type, ">::string_literal("sv);
    constexpr char HexDigit[] = "0123456789ABCDEF";
    b.append('"');
    const auto *p = reinterpret_cast<const u8 *>(array.data());
    for (auto i = 0; i < array.size() * sizeof(T); ++i) {
        b.append(R"(\x)"sv);
        b.append(HexDigit[p[i] / 16]);
        b.append(HexDigit[p[i] % 16]);
    }
    b.append('"');
    Formatting::format_into(b, ","sv, array.size(), ")"sv);
}

void Formatter<sigil::StaticTable>::format(
    StringBuilder &b, const sigil::StaticTable &table)
{
    Formatting::format_into(b, "({"sv);

    Formatting::format_into(b, "const auto transitions = "sv);
    format_sigil_array(b, "sigil::State"sv, table.transitions());
    Formatting::format_into(b, ";"sv);

    Formatting::format_into(b, "const auto accepting = "sv);
    format_sigil_array(b, "sigil::TokenType"sv, table.accepting());
    Formatting::format_into(b, ";"sv);

    Formatting::format_into(
        b,
        "sigil::StaticTable("sv,
        table.start_state(),
        ","sv,
        table.error_state(),
        ",transitions,accepting);"sv);
    Formatting::format_into(b, "})"sv);
}

}  // namespace core