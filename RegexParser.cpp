//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "RegexParser.h"

#include <core/Formatting.h>
#include <sigil/RegExp.h>

namespace sigil {

void sigil::RegexParser::initialize(StringView input)
{
    m_input = input;
    m_offset = 0;
}

//
// regexp ::= <atom>
//          | <regexp> | <regexp>
//          | <regexp> <regexp>
//          | <regexp> *
//          | <regexp> +
//          | <regexp> ?
//          | ( <regexp> )
//          ;
//
// atom ::= CHAR
//        ;
//

template<typename T, typename... Args>
inline static RegExp *create_reg_exp(Args... args)
{
    return new T(std::forward<Args>(args)...);
}

Either<StringView, RegExp *> sigil::RegexParser::parse()
{
    using Result = Either<StringView, RegExp *>;

    if (auto exp = parse_alternative()) {
        if (can_peek())
            return Result::left("Non-exhaustive parse");
        return Result::right(exp);
    }

    return Result::left("Parse error");
}

inline static bool between(char min, char c, char max)
{
    return min <= c and c <= max;
}
inline static bool can_be_atom(char c)
{
    if (between('\0', c, '\u001F'))
        return false;
    if (between(' ', c, '\''))
        return true;
    if (between('(', c, '+'))
        return false;
    if (between('-', c, '{'))
        return true;
    if (c == '|')
        return false;
    if (between('}', c, '~'))
        return true;
    return false;
}
inline static bool can_be_primary_expression(char c)
{
    if (can_be_atom(c))
        return true;
    if (c == '(')
        return true;
    return false;
}

RegExp *RegexParser::parse_alternative()
{
    auto result = parse_concatenation();
    if (result == nullptr)
        return nullptr;

    while (can_peek() and peek() == '|') {
        assert(advance() == '|');
        auto exp = parse_concatenation();
        if (exp == nullptr)
            return nullptr;

        result = create_reg_exp<Alternative>(result, exp);
    }

    return result;
}

RegExp *RegexParser::parse_concatenation()
{
    auto result = parse_postfix();
    if (result == nullptr)
        return nullptr;

    while (can_peek() and can_be_primary_expression(peek())) {
        auto exp = parse_postfix();
        if (exp == nullptr)
            return nullptr;

        result = create_reg_exp<Concatenation>(result, exp);
    }

    return result;
}

RegExp *RegexParser::parse_postfix()
{
    auto result = parse_primary();
    if (result == nullptr)
        return nullptr;

    const auto is_postfix_operator = [](char c) {
        return '*' == c or '+' == c or '?' == c;
    };

    while (can_peek() and is_postfix_operator(peek())) {
        switch (advance()) {
            case '*': result = create_reg_exp<Kleene>(result); break;
            case '+': result = create_reg_exp<PositiveKleene>(result); break;
            case '?': result = create_reg_exp<Optional>(result); break;
            default: assert(false and "Unreachable");
        }
    }

    return result;
}

bool RegexParser::unescape(char &result, char c)
{
    switch (c) {
        case '\\': result = '\\'; return true;
        case 't': result = '\t'; return true;
        case 'r': result = '\r'; return true;
        case 'n': result = '\n'; return true;
        default: return false;
    }
}

bool RegexParser::escape(StringView &result, char c)
{
    switch (c) {
        case '\\': result = "\\\\"; return true;
        case '\t': result = "\\t"; return true;
        case '\r': result = "\\r"; return true;
        case '\n': result = "\\n"; return true;
        default: return false;
    }
}

RegExp *RegexParser::parse_primary()
{
    if (can_be_atom(peek())) {
        auto c = advance();
        if (c == '\\') {
            if (RegexParser::unescape(c, peek()))
                advance();
        }
        return create_reg_exp<Atom>(c);
    }

    if (peek() == '(') {
        advance();
        auto exp = parse_alternative();
        if (peek() != ')') {
            debug_log("Expected ), but got `", peek(), "`\n");
            return nullptr;
        }
        advance();
        return exp;
    }

    debug_log("Expected atom, but got `", peek(), "`\n");
    return nullptr;
}

bool RegexParser::can_peek() const
{
    if (m_offset < 0)
        return false;
    return m_offset < m_input.size();
}

char RegexParser::peek()
{
    assert(can_peek());
    return m_input[m_offset];
}

char RegexParser::advance()
{
    const auto c = peek();
    ++m_offset;
    return c;
}

}  // namespace sigil
