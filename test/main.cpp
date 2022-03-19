//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <core/Formatting.h>

#include <sigil/DfaSimulation.h>
#include <sigil/DfaTableScannerDriver.h>
#include <sigil/RegExp.h>
#include <sigil/RegexParser.h>
#include <sigil/ScannerDriver.h>
#include <sigil/SpecialTokenType.h>

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
    core::Arena arena;
    sigil::RegexParser parser(arena);
    parser.initialize(regex_pattern);
    auto either_exp = parser.parse();
    if (not either_exp.isRight())
        return core::Formatting::format("Parse error: ", either_exp.left());

    const auto regexp = either_exp.release_right();
    return core::Formatting::format(*regexp);
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
    expect_eq(parse_regex("[ab]"), "Atom('a' - 'b')");
    expect_eq(parse_regex("[a-c]"), "Atom('a' - 'c')");
    expect_eq(parse_regex("[a-zA-Z]"), "Atom('A' - 'Z', 'a' - 'z')");

    expect_eq(parse_regex("[-a]"), "Atom('-', 'a')");
    expect_eq(parse_regex("[^\\u00-/:-\\uFF]"), "Atom('0' - '9')");
}

static void dfa_simulation_tests_calculator()
{
    enum class Type : s32
    {
        Eof = s32(sigil::SpecialTokenType::Eof),
        Error = s32(sigil::SpecialTokenType::Error),
        Plus,
        Star,
        OpenParenthesis,
        CloseParenthesis,
        Literal,
        Identifier,
        Whitespace,
    };
    sigil::Specification spec;
    spec.add_literal_token((s32)Type::Plus, "Plus", "+");
    spec.add_literal_token((s32)Type::Star, "Star", "*");
    spec.add_literal_token((s32)Type::OpenParenthesis, "OpenParenthesis", "(");
    spec.add_literal_token(
        (s32)Type::CloseParenthesis, "CloseParenthesis", ")");
    spec.add_regex_token((s32)Type::Literal, "Literal", "[0-9]+");
    spec.add_regex_token(
        (s32)Type::Identifier, "Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
    spec.add_regex_token((s32)Type::Whitespace, "Whitespace", "[ \\n\\r\\t]+");

    auto either_grammar = sigil::Grammar::compile(spec);
    if (not either_grammar.isRight()) {
        debug_log("Error: ", either_grammar.left(), "\n");
        return;
    }

    auto grammar = std::move(either_grammar.release_right());

    using namespace sigil::dfa;

    expect_eq(simulate(grammar, "+"), SimulationResult::accept("Plus"));
    expect_eq(simulate(grammar, "*"), SimulationResult::accept("Star"));
    expect_eq(
        simulate(grammar, "("), SimulationResult::accept("OpenParenthesis"));
    expect_eq(
        simulate(grammar, ")"), SimulationResult::accept("CloseParenthesis"));

    expect_eq(simulate(grammar, " "), SimulationResult::accept("Whitespace"));
    expect_eq(simulate(grammar, "  "), SimulationResult::accept("Whitespace"));
    expect_eq(simulate(grammar, "\n"), SimulationResult::accept("Whitespace"));
    expect_eq(
        simulate(grammar, "\n\r"), SimulationResult::accept("Whitespace"));
    expect_eq(
        simulate(grammar, "\r\n"), SimulationResult::accept("Whitespace"));
    expect_eq(simulate(grammar, "\t"), SimulationResult::accept("Whitespace"));

    expect_eq(simulate(grammar, "0"), SimulationResult::accept("Literal"));
    expect_eq(simulate(grammar, "1"), SimulationResult::accept("Literal"));
    expect_eq(simulate(grammar, "10"), SimulationResult::accept("Literal"));
    expect_eq(simulate(grammar, "9999"), SimulationResult::accept("Literal"));
    expect_eq(simulate(grammar, "12345"), SimulationResult::accept("Literal"));

    expect_eq(simulate(grammar, "if"), SimulationResult::accept("Identifier"));
    expect_eq(simulate(grammar, "ifx"), SimulationResult::accept("Identifier"));
    expect_eq(simulate(grammar, "abc"), SimulationResult::accept("Identifier"));
    expect_eq(
        simulate(grammar, "my_list"), SimulationResult::accept("Identifier"));
    expect_eq(
        simulate(grammar, "Test_3"), SimulationResult::accept("Identifier"));
}

static void dfa_simulation_tests_conflict()
{
    enum class Type : s32
    {
        Eof = s32(sigil::SpecialTokenType::Eof),
        Error = s32(sigil::SpecialTokenType::Error),
        KwIf,
        Identifier,
    };
    sigil::Specification spec;
    spec.add_literal_token((s32)Type::KwIf, "KwIf", "if");
    spec.add_regex_token(
        (s32)Type::Identifier, "Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");

    auto either_grammar = sigil::Grammar::compile(spec);
    if (not either_grammar.isRight()) {
        debug_log("Error: ", either_grammar.left(), "\n");
        return;
    }

    auto grammar = std::move(either_grammar.release_right());

    using namespace sigil::dfa;

    expect_eq(simulate(grammar, "if"), SimulationResult::accept("KwIf"));
    expect_eq(simulate(grammar, "ifx"), SimulationResult::accept("Identifier"));
    expect_eq(simulate(grammar, "abc"), SimulationResult::accept("Identifier"));
    expect_eq(
        simulate(grammar, "my_list"), SimulationResult::accept("Identifier"));
    expect_eq(
        simulate(grammar, "Test_3"), SimulationResult::accept("Identifier"));
}

static void scanner_detect_eof_instead_of_error()
{
    enum class TokenType : int8_t
    {
        Eof = int8_t(sigil::SpecialTokenType::Eof),
        Error = int8_t(sigil::SpecialTokenType::Error),
        Word,
        QMark,
    };

    auto show = [](int type) -> StringView {
        switch (TokenType(type)) {
            case TokenType::Eof: return "Eof"sv;
            case TokenType::Error: return "Error"sv;
            case TokenType::Word: return "Word"sv;
            case TokenType::QMark: return "QMark"sv;
        }
        assert(false and "Unreachable");
        return {};
    };

    {
        sigil::Specification specification;
        specification.add_regex_token(
            (s32)TokenType::Word, "Word", "[-a-zA-Z/]+");
        specification.add_literal_token((s32)TokenType::QMark, "QMark", "?");
        auto either_grammar = sigil::Grammar::compile(specification);
        auto grammar = std::move(either_grammar.release_right());
        auto scanner = sigil::DfaTableScannerDriver::create(grammar.dfa());
        scanner.initialize("<string>", "hello?");
        expect_eq(show(scanner.next().type), "Word"sv);
        expect_eq(show(scanner.next().type), "QMark"sv);
        expect_eq(show(scanner.next().type), "Eof"sv);
    }

    {
        sigil::Specification specification;
        specification.add_regex_token(
            (s32)TokenType::Word, "Word", "[-a-zA-Z/]+");
        auto either_grammar = sigil::Grammar::compile(specification);
        auto grammar = std::move(either_grammar.release_right());
        auto scanner = sigil::DfaTableScannerDriver::create(grammar.dfa());
        scanner.initialize("<string>", "hello?");
        expect_eq(show(scanner.next().type), "Word"sv);
        expect_eq(show(scanner.next().type), "Error"sv);
    }
}

static void user_controlled_token_values()
{
    sigil::Specification specification;
    specification.add_literal_token(1, "A", "a");
    specification.add_literal_token(42, "B", "b");
    specification.add_literal_token(55, "C", "c");

    auto either_grammar = sigil::Grammar::compile(specification);
    auto grammar = std::move(either_grammar.release_right());
    auto scanner = sigil::DfaTableScannerDriver::create(grammar.dfa());
    scanner.initialize("<string>", "abc");
    expect_eq(scanner.next().type, 1);
    expect_eq(scanner.next().type, 42);
    expect_eq(scanner.next().type, 55);
    expect_eq(scanner.next().type, (s32)sigil::SpecialTokenType::Eof);
}

void sigil_tests()
{
    regex_parser_tests();
    dfa_simulation_tests_calculator();
    dfa_simulation_tests_conflict();
    scanner_detect_eof_instead_of_error();
    user_controlled_token_values();
}

int main()
{
    sigil_tests();
    return EXIT_SUCCESS;
}