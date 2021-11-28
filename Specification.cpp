//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Specification.h"

#include <core/Logging.h>

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

void Logger<sigil::Specification::TokenSpec::Type>::log(
    sigil::Specification::TokenSpec::Type value)
{
    switch (value) {
        case sigil::Specification::TokenSpec::Type::Literal:
            Logging::log("Literal");
            break;
        case sigil::Specification::TokenSpec::Type::Regex:
            Logging::log("Regex");
            break;
        case sigil::Specification::TokenSpec::Type::Invalid:
        default: assert(false and "Unreachable");
    }
}

void Logger<sigil::Specification::TokenSpec>::log(
    const sigil::Specification::TokenSpec &token_spec)
{
    Logging::log(
        "TokenSpec( ",
        token_spec.type,
        ", ",
        token_spec.name,
        ", ",
        token_spec.pattern,
        " )");
}

}  // namespace core