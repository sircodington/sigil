//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/Specification.h>

#include <core/Formatting.h>

namespace sigil {

void Specification::add_literal_token(
    s32 token_type, StringView token_name, StringView exact_string)
{
    auto token = TokenSpec::literal(token_type, token_name, exact_string);
    m_tokens.add(std::move(token));
}

void Specification::add_regex_token(
    s32 token_type, StringView token_name, StringView regex)
{
    auto token = TokenSpec::regex(token_type, token_name, regex);
    m_tokens.add(std::move(token));
}

void Specification::add_nfa_token(
    s32 token_type,
    StringView token_name,
    std::function<void(sigil::nfa::Automaton &)> build)
{
    auto token = TokenSpec::nfa(token_type, token_name, std::move(build));
    m_tokens.add(std::move(token));
}

Specification::TokenSpec Specification::TokenSpec::literal(
    s32 token_type, StringView token_name, StringView literal)
{
    return { Type::Literal, token_type, token_name, literal, {} };
}

Specification::TokenSpec Specification::TokenSpec::regex(
    s32 token_type, StringView token_name, StringView regex)
{
    return { Type::Regex, token_type, token_name, regex, {} };
}

Specification::TokenSpec Specification::TokenSpec::nfa(
    s32 token_type,
    StringView token_name,
    std::function<void(sigil::nfa::Automaton &)> build)
{
    return { Type::Nfa, token_type, token_name, {}, std::move(build) };
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
        case sigil::Specification::TokenSpec::Type::Nfa:
            Formatting::format_into(b, "Nfa");
            break;
        case sigil::Specification::TokenSpec::Type::Invalid:
        default: assert(false and "Unreachable");
    }
}

void Formatter<sigil::Specification::TokenSpec>::format(
    StringBuilder &b, const sigil::Specification::TokenSpec &token_spec)
{
    Formatting::format_into(
        b, "TokenSpec('"sv, token_spec.name, "', "sv, token_spec.type);
    switch (token_spec.type) {
        case sigil::Specification::TokenSpec::Type::Invalid: break;
        case sigil::Specification::TokenSpec::Type::Literal: [[fallthrough]];
        case sigil::Specification::TokenSpec::Type::Regex:
            Formatting::format_into(b, "`"sv, token_spec.pattern, "`"sv);
            break;
        case sigil::Specification::TokenSpec::Type::Nfa:
            Formatting::format_into(b, "<function>"sv, "`"sv);
            break;
    }
    Formatting::format_into(b, ")"sv);
}

}  // namespace core