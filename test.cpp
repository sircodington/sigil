//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "test.h"

#include <core/Formatting.h>
#include <sigil/RegExp.h>
#include <sigil/RegexParser.h>

#define expect_eq(actual, expect)                  \
    do {                                           \
        const auto x = (actual);                   \
        const auto y = (expect);                   \
        if (x != y) {                              \
            debug_log(                             \
                "Expected equality of values\n  ", \
                #actual,                           \
                "\n  ",                            \
                x,                                 \
                "\n and\n  ",                      \
                #expect,                           \
                "\n  ",                            \
                y,                                 \
                "\n\n");                           \
        }                                          \
    } while (0)

String parse_regex(const StringView &regex_pattern)
{
    sigil::RegexParser parser;
    parser.initialize(regex_pattern);
    auto either_exp = parser.parse();
    if (not either_exp.isRight())
        return compose("Parse error: ", either_exp.left());

    const auto regexp = either_exp.release_right();
    return compose(*regexp);
}

static void regex_parser_tests()
{
    expect_eq(parse_regex("a"), "Atom('a')");
    expect_eq(parse_regex(" "), "Atom(' ')");
    expect_eq(parse_regex("\\n"), "Atom('\\n')");
    expect_eq(parse_regex("\\u5E"), "Atom('^')");

    expect_eq(parse_regex("a|b"), "Alternative(Atom('a'), Atom('b'))");
    expect_eq(parse_regex("a| "), "Alternative(Atom('a'), Atom(' '))");
    expect_eq(parse_regex("a|\\n"), "Alternative(Atom('a'), Atom('\\n'))");
    expect_eq(parse_regex("a|\\u5E"), "Alternative(Atom('a'), Atom('^'))");

    expect_eq(parse_regex("aa"), "Concatenation(Atom('a'), Atom('a'))");
    expect_eq(parse_regex("a "), "Concatenation(Atom('a'), Atom(' '))");
    expect_eq(parse_regex("\\\\n"), "Concatenation(Atom('\\\\'), Atom('n'))");
    expect_eq(parse_regex("a\\n"), "Concatenation(Atom('a'), Atom('\\n'))");
    expect_eq(parse_regex("a\\u5E"), "Concatenation(Atom('a'), Atom('^'))");

    expect_eq(parse_regex("a*"), "Kleene(Atom('a'))");
    expect_eq(parse_regex("a+"), "PositiveKleene(Atom('a'))");
    expect_eq(parse_regex("a?"), "Optional(Atom('a'))");
    expect_eq(
        parse_regex("a*+?"), "Optional(PositiveKleene(Kleene(Atom('a'))))");

    expect_eq(
        parse_regex("ab|c"),
        "Alternative(Concatenation(Atom('a'), Atom('b')), Atom('c'))");
    expect_eq(
        parse_regex("a|bc"),
        "Alternative(Atom('a'), Concatenation(Atom('b'), Atom('c')))");

    expect_eq(parse_regex("(a)"), "Atom('a')");
    expect_eq(
        parse_regex("a(b|c)"),
        "Concatenation(Atom('a'), Alternative(Atom('b'), Atom('c')))");

    // @NOTE: The parser accepts the empty character class, it shall be handled
    // elsewhere
    expect_eq(parse_regex("[]"), "Atom()");

    expect_eq(parse_regex("[a]"), "Atom('a')");
    expect_eq(parse_regex("[ab]"), "Atom('a','b')");
    expect_eq(parse_regex("[a-c]"), "Atom('a','b','c')");
    expect_eq(
        parse_regex("[a-zA-Z]"),
        "Atom('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',"
        "'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g',"
        "'h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x',"
        "'y','z')");

    expect_eq(parse_regex("[-a]"), "Atom('-','a')");
}

void sigil_tests() { regex_parser_tests(); }
