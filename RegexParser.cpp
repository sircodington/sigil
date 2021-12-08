//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "RegexParser.h"

#include <algorithm>
#include <cctype>

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
    if (between('-', c, 'Z'))
        return true;
    if (c == '[')
        return false;
    if (c == '\\')
        return true;
    if (c == ']')
        return false;
    if (between('^', c, '{'))
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
    if (c == '[')
        return true;
    return false;
}
inline static bool unhex(u8 &result, char c)
{
    if (not(between('a', c, 'f') or between('A', c, 'F') or
            between('0', c, '9')))
        return false;

    c = char(tolower(c));
    if (between('0', c, '9')) {
        result = c - '0';
    } else {
        assert(between('a', c, 'f'));
        result = 10 + c - 'a';
    }
    return true;
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

bool RegexParser::unescape(
    u8 &result, u8 &advance, const core::StringView &view)
{
    if (view.is_empty())
        return false;

    switch (view[0]) {
        case '\\':
            result = '\\';
            advance = 1;
            return true;
        case 't':
            result = '\t';
            advance = 1;
            return true;
        case 'r':
            result = '\r';
            advance = 1;
            return true;
        case 'n':
            result = '\n';
            advance = 1;
            return true;
        case 'u': {
            if (view.size() < 3)
                return false;

            u8 d0, d1;
            if (not unhex(d0, view[1]))
                return false;
            if (not unhex(d1, view[2]))
                return false;

            advance = 3;
            result = u8(d0 * 16 + d1);
            return true;
        }
        default: return false;
    }
}

String RegexParser::escape(char c)
{
    switch (c) {
        case '\\': return StringView("\\\\");
        case '\t': return StringView("\\t");
        case '\r': return StringView("\\r");
        case '\n': return StringView("\\n");
        default:
            if (between(' ', c, '~')) {
                return StringView(&c, 1);
            } else {
                char buffer[11] { 0 };
                buffer[0] = 'u';
                auto count = snprintf(buffer + 1, sizeof(buffer) - 1, "%X", c);
                assert(count >= 0 and "snprintf failed");
                return StringView(buffer, count + 1);
            }
    }
}

constexpr static auto ErrorAtom = std::numeric_limits<uint64_t>::max();

uint64_t RegexParser::parse_atom()
{
    assert(can_be_atom(peek()));
    u8 c = advance();
    if (c == '\\') {
        const core::StringView peeked(
            m_input.data() + m_offset,
            std::min(m_input.size() - Size(m_offset), Size(3)));

        u8 skip = 0;
        if (not RegexParser::unescape(c, skip, peeked)) {
            debug_log("Invalid escape sequence\n");
            return ErrorAtom;
        }
        for (u8 i = 0; i < skip; ++i) advance();
    }
    return c;
}

RegExp *RegexParser::parse_primary()
{
    if (can_be_atom(peek())) {
        auto atom = parse_atom();
        if (ErrorAtom == atom)
            return nullptr;

        CharSet singleton_char_set { u8(atom) };
        return create_reg_exp<Atom>(std::move(singleton_char_set));
    }

    if (peek() == '[') {
        advance();

        const auto negate = peek() == '^';
        if (negate)
            advance();

        CharSet char_set;
        while (peek() != ']') {
            auto curr = parse_atom();
            if (ErrorAtom == curr)
                return nullptr;

            if (peek() == '-') {
                advance();

                auto next = parse_atom();
                if (ErrorAtom == next)
                    return nullptr;

                char_set.set(u8(curr), u8(next), true);
            } else {
                char_set.set(u8(curr), true);
            }
        }
        advance();

        if (negate)
            char_set.negate();

        return create_reg_exp<Atom>(std::move(char_set));
    }

    if (peek() == '(') {
        advance();
        auto exp = parse_alternative();
        if (peek() != ')') {
            debug_log("Expected `)`, but got `", peek(), "`\n");
            return nullptr;
        }
        advance();
        return exp;
    }

    debug_log("Expected primary expression, but got `", peek(), "`\n");
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
