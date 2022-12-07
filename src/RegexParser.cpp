//
// Copyright (c) 2021-2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/RegexParser.h>

#include <cctype>

#include <core/Formatting.h>

#include <sigil/RegExp.h>

namespace sigil {

RegexParser::RegexParser(core::Arena &arena)
    : m_arena(arena)
{
}

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
//        | CHAR-CLASS
//        ;
//

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

inline static bool between(u8 min, u8 c, u8 max)
{
    return min <= c and c <= max;
}

inline static bool can_be_class_atom(u8 c);
inline static bool can_be_top_level_atom(u8 c);
inline static bool can_be_atom(u8 c);

inline static bool unhex(u8 &result, u8 c)
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
    if (not result)
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
    if (not result)
        return nullptr;

    while (can_peek() and can_be_atom(peek())) {
        auto exp = parse_postfix();
        if (exp == nullptr)
            return nullptr;

        result = create_reg_exp<Concatenation>(result, exp);
    }

    return result;
}

RegExp *RegexParser::parse_postfix()
{
    auto result = parse_atom();
    if (not result)
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

inline static bool can_be_atom(u8 c)
{
    if (c == '(' or c == '[')
        return true;
    return can_be_top_level_atom(c);
}

RegExp *RegexParser::parse_atom()
{
    assert(can_be_atom(peek()));
    if (peek() == '(')
        return parse_nested_atom();
    if (peek() == '[')
        return parse_class_atom();
    return parse_top_level_atom();
}

RegExp *RegexParser::parse_nested_atom()
{
    assert(peek() == '(');
    advance();  // '('
    auto exp = parse_alternative();
    if (peek() != ')') {
        debug_log("Expected `)`, but got `", peek(), "`\n");
        return nullptr;
    }
    advance();
    return exp;
}

RegExp *RegexParser::parse_class_atom()
{
    assert(peek() == '[');
    advance();  // '['

    const auto negate = peek() == '^';
    if (negate)
        advance();

    CharSet char_set;
    while (peek() != ']') {
        auto curr = parse_class_chars();
        if (curr.is_empty())
            return nullptr;
        char_set |= curr;
    }
    advance();  // ']'

    if (negate)
        char_set.negate();

    return create_reg_exp<Atom>(std::move(char_set));
}

RegExp *RegexParser::parse_top_level_atom()
{
    assert(can_be_top_level_atom(peek()));
    auto char_set = parse_top_level_chars();
    if (char_set.is_empty())
        return nullptr;
    return create_reg_exp<Atom>(std::move(char_set));
}

inline static bool can_be_top_level_atom(u8 c)
{
    if (between('a', c, 'z'))
        return true;
    if (between('A', c, 'Z'))
        return true;
    if (between('0', c, '9'))
        return true;

    switch (c) {
        case '.':
        case '\\':
        case ' ':
        case '-':
        case ':':
        case '/':
        case '_':
        case '^':
        case '$':
        case '%':
        case '&':
        case '!':
        case '#':
        case '\'':
        case '\n':
        case '\r':
        case '\t': return true;
        default: return false;
    }
}

CharSet RegexParser::parse_top_level_chars()
{
    assert(can_be_top_level_atom(peek()));

    if (peek() == '.') {
        advance();  // '.'
        return ~CharSet();
    }
    if (peek() == '-') {
        advance();  // '-'
        return CharSet('-');
    }
    if (peek() == '^') {
        assert(false and "Anchors are not implemented!");
    }
    if (peek() == '$') {
        assert(false and "Anchors are not implemented!");
    }
    if (peek() == '%') {
        advance();  // '%'
        return CharSet('%');
    }
    if (peek() == '&') {
        advance();  // '&'
        return CharSet('&');
    }
    if (peek() == '!') {
        advance();  // '!'
        return CharSet('!');
    }
    if (peek() == '#') {
        advance();  // '#'
        return CharSet('#');
    }
    if (peek() == '\'') {
        advance();  // '\''
        return CharSet('\'');
    }
    if (peek() == '\n') {
        advance();  // '\n'
        return CharSet('\n');
    }
    if (peek() == '\r') {
        advance();  // '\r'
        return CharSet('\r');
    }
    if (peek() == '\t') {
        advance();  // '\t'
        return CharSet('\t');
    }

    if (peek() == '\\') {
        advance();  // '\\'
        if (not can_peek())
            return {};

        static CharSet Digit('0', '9');
        static auto Word =
            CharSet('a', 'z') | CharSet('A', 'Z') | Digit | CharSet('_');
        static auto WhiteSpace = CharSet('\r') | CharSet('\n') | CharSet('\t') |
                                 CharSet('\f') | CharSet('\v') | CharSet(' ');

        switch (advance()) {
            case '|': return CharSet('|');
            case '.': return CharSet('.');
            case '\\': return CharSet('\\');
            case 't': return CharSet('\t');
            case 'r': return CharSet('\r');
            case 'n': return CharSet('\n');
            case '^': return CharSet('^');
            case '$': return CharSet('$');
            case '%': return CharSet('%');
            case '&': return CharSet('&');
            case '+': return CharSet('+');
            case '!': return CharSet('!');
            case '#': return CharSet('#');
            case '\'': return CharSet('\'');
            case 'd': return Digit;
            case 'D': return ~Digit;
            case 'w': return Word;
            case 'W': return ~Word;
            case 's': return WhiteSpace;
            case 'S': return ~WhiteSpace;
            case 'u': {
                if (not can_peek())
                    return {};
                const auto a = advance();

                if (not can_peek())
                    return {};
                const auto b = advance();

                u8 d0, d1;
                if (not unhex(d0, a))
                    return {};
                if (not unhex(d1, b))
                    return {};
                return CharSet(u8(d0 * 16 + d1));
            }
            default: return {};
        }
    }

    return CharSet(advance());
}

inline static bool can_be_class_atom(u8 c)
{
    if (between('a', c, 'z'))
        return true;
    if (between('A', c, 'Z'))
        return true;
    if (between('0', c, '9'))
        return true;

    switch (c) {
        case '.':
        case '\\':
        case ' ':
        case '-':
        case ':':
        case '/':
        case '_':
        case '^':
        case '$':
        case '%':
        case '&':
        case '+':
        case '!':
        case '#':
        case '\'':
        case '\n':
        case '\r':
        case '\t': return true;
        default: return false;
    }
}

CharSet RegexParser::parse_class_chars()
{
    struct Result
    {
        Result() = default;
        explicit Result(u8 value)
            : ok(true)
            , value(value)
        {
        }
        bool ok { false };
        u8 value { 0 };
    };

    const auto parse_class_char = [this]() -> Result {
        if (not can_peek() or not can_be_class_atom(peek()))
            return {};

        if (peek() == '.') {
            advance();  // '.'
            return Result('.');
        }
        if (peek() == '^') {
            advance();  // '^'
            return Result('^');
        }
        if (peek() == '$') {
            advance();  // '$'
            return Result('$');
        }
        if (peek() == '%') {
            advance();  // '%'
            return Result('%');
        }
        if (peek() == '&') {
            advance();  // '&'
            return Result('&');
        }
        if (peek() == '+') {
            advance();  // '+'
            return Result('+');
        }
        if (peek() == '!') {
            advance();  // '!'
            return Result('!');
        }
        if (peek() == '#') {
            advance();  // '#'
            return Result('#');
        }
        if (peek() == '\'') {
            advance();  // '\''
            return Result('\'');
        }
        if (peek() == '\n') {
            advance();  // '\n'
            return Result('\n');
        }
        if (peek() == '\r') {
            advance();  // '\r'
            return Result('\r');
        }
        if (peek() == '\t') {
            advance();  // '\t'
            return Result('\t');
        }

        if (peek() == '\\') {
            advance();  // '\\'

            if (not can_peek())
                return {};

            switch (advance()) {
                case '|': return Result('|');
                case '.': return Result('.');
                case '\\': return Result('\\');
                case 't': return Result('\t');
                case 'r': return Result('\r');
                case 'n': return Result('\n');
                case '^': return Result('^');
                case '$': return Result('$');
                case '%': return Result('%');
                case '&': return Result('&');
                case '+': return Result('+');
                case '!': return Result('!');
                case '#': return Result('#');
                case '\'': return Result('\'');
                case 'd':
                case 'D':
                case 'w':
                case 'W':
                case 's':
                case 'S':
                    // @NOTE: Illegal escape sequence (not allowed in character
                    // classes)
                    return {};
                case 'u': {
                    if (not can_peek())
                        return {};
                    const auto a = advance();

                    if (not can_peek())
                        return {};
                    const auto b = advance();

                    u8 d0, d1;
                    if (not unhex(d0, a))
                        return {};
                    if (not unhex(d1, b))
                        return {};
                    return Result(u8(d0 * 16 + d1));
                }
            }
        }

        return Result(advance());
    };

    if (peek() == '-') {
        advance();  //'-'
        return CharSet('-');
    }

    if (auto a = parse_class_char(); a.ok) {
        if (peek() == '-') {
            advance();  // '-'
            if (peek() == ']')
                return CharSet(a.value) | CharSet('-');

            if (auto b = parse_class_char(); b.ok)
                return { a.value, b.value };

            // @NOTE: Unexpected character in class: expected ']' or
            // `class_char`
            return {};
        }

        return CharSet(a.value);
    }

    return {};
}

bool RegexParser::can_peek() const
{
    if (m_offset < 0)
        return false;
    return m_offset < m_input.size();
}

u8 RegexParser::peek()
{
    assert(can_peek());
    return m_input[m_offset];
}

u8 RegexParser::advance()
{
    const auto c = peek();
    ++m_offset;
    return c;
}

}  // namespace sigil
