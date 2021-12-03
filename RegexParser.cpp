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
inline static bool is_atom(char c)
{
    if (between('a', c, 'z'))
        return true;
    if (between('A', c, 'Z'))
        return true;
    if (between('0', c, '9'))
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

    while (can_peek() and is_atom(peek())) {
        auto exp = parse_postfix();
        if (exp == nullptr)
            return nullptr;

        result = create_reg_exp<Concatenation>(result, exp);
    }

    return result;
}

RegExp *RegexParser::parse_postfix()
{
    auto atom = parse_atom();
    if (atom == nullptr)
        return nullptr;

    if (not can_peek())
        return atom;

    switch (peek()) {
        case '*': advance(); return create_reg_exp<Kleene>(atom);
        case '+': advance(); return create_reg_exp<PositiveKleene>(atom);
        case '?': advance(); return create_reg_exp<Optional>(atom);
        default: return atom;
    }
}

RegExp *RegexParser::parse_atom()
{
    const auto c = peek();
    if (is_atom(c)) {
        advance();
        return create_reg_exp<Atom>(c);
    }

    debug_log("Expected atom, but got `", c, "`\n");
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
