//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/Specification.h>

#include <core/Formatting.h>

namespace sigil {

void Specification::add_literal_token(
    StringView token_name, StringView exact_string)
{
    TokenSpec token {
        TokenSpec::Type::Literal,
        token_name,
        exact_string,
    };
    m_tokens.add(token);
}

void Specification::add_regex_token(StringView token_name, StringView regex)
{
    TokenSpec token {
        TokenSpec::Type::Regex,
        token_name,
        regex,
    };
    m_tokens.add(token);
}

}  // namespace sigil

namespace core {

void Formatter<sigil::Specification::TokenSpec::Type>::format(
    StringBuilder &b, sigil::Specification::TokenSpec::Type value)
{
    switch (value) {
        case sigil::Specification::TokenSpec::Type::Literal:
            Formatting::format_into(b, "Literal");
            break;
        case sigil::Specification::TokenSpec::Type::Regex:
            Formatting::format_into(b, "Regex");
            break;
        case sigil::Specification::TokenSpec::Type::Invalid:
        default: assert(false and "Unreachable");
    }
}

void Formatter<sigil::Specification::TokenSpec>::format(
    StringBuilder &b, const sigil::Specification::TokenSpec &token_spec)
{
    Formatting::format_into(
        b,
        "TokenSpec( ",
        token_spec.type,
        ", ",
        token_spec.name,
        ", ",
        token_spec.pattern,
        " )");
}

}  // namespace core