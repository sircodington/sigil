//
// Copyright (c) 2021-2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <core/Formatting.h>
#include <core/Test.h>

#include <sigil/CharSet.h>
#include <sigil/DfaSimulation.h>
#include <sigil/DfaTableScannerDriver.h>
#include <sigil/RegExp.h>
#include <sigil/RegexParser.h>

static void char_set_tests()
{
    {
        sigil::CharSet empty;
        assert(empty.is_empty());
    }

    {
        auto complete = ~sigil::CharSet();
        assert(complete.non_empty());
        for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
            assert(complete.contains(i));
        }
    }

    assert(sigil::CharSet() == sigil::CharSet());
    assert(sigil::CharSet('x') == sigil::CharSet('x'));
    assert(sigil::CharSet('0', '9') == sigil::CharSet('0', '9'));

    const auto set = [](u8 first, u8 last) {
        return sigil::CharSet(first, last);
    };
    assert((set('a', 's') | set('k', 'z')) == set('a', 'z'));
    assert((set('a', 's') & set('k', 'z')) == set('k', 's'));
    assert((set('a', 's') / set('k', 'z')) == set('a', 'j'));
}

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

    expect_eq(parse_regex("\\d"), "Atom('0' - '9')");
    // @FIXME: Replace be real error, once we properly propagate failure
    expect_eq(parse_regex("[\\d]"), "Parse error: Parse error");

    expect_eq(parse_regex("\\D"), "Atom('\\u0' - '/', ':' - '\\uFF')");
    // @FIXME: Replace be real error, once we properly propagate failure
    expect_eq(parse_regex("[\\D]"), "Parse error: Parse error");

    expect_eq(parse_regex("\\w"), "Atom('0' - '9', 'A' - 'Z', '_', 'a' - 'z')");
    // @FIXME: Replace be real error, once we properly propagate failure
    expect_eq(parse_regex("[\\w]"), "Parse error: Parse error");

    expect_eq(
        parse_regex("\\W"),
        "Atom('\\u0' - '/', ':' - '@', '[' - '^', '`', '{' - '\\uFF')");
    // @FIXME: Replace be real error, once we properly propagate failure
    expect_eq(parse_regex("[\\W]"), "Parse error: Parse error");

    expect_eq(parse_regex("\\s"), "Atom('\\t' - '\\r', ' ')");
    // @FIXME: Replace be real error, once we properly propagate failure
    expect_eq(parse_regex("[\\s]"), "Parse error: Parse error");

    expect_eq(
        parse_regex("\\S"),
        "Atom('\\u0' - '\\u8', '\\uE' - '\\u1F', '!' - '\\uFF')");
    // @FIXME: Replace be real error, once we properly propagate failure
    expect_eq(parse_regex("[\\S]"), "Parse error: Parse error");

    // Floating point numbers
    expect_eq(
        parse_regex(R"END(([eE][+-]?\d+)?)END"sv),
        "Optional(Concatenation(Concatenation(Atom('E', 'e'), "
        "Optional(Atom('+', '-'))), PositiveKleene(Atom('0' - '9'))))"sv);
    expect_eq(
        parse_regex(R"END((\d+(\.\d*)?|\d*\.\d+)([eE][+-]?\d+)?[#!]?)END"sv),
        "Concatenation(Concatenation(Alternative(Concatenation(PositiveKleene("
        "Atom('0' - '9')), Optional(Concatenation(Atom('.'), Kleene(Atom('0' - "
        "'9'))))), Concatenation(Concatenation(Kleene(Atom('0' - '9')), "
        "Atom('.')), PositiveKleene(Atom('0' - '9')))), "
        "Optional(Concatenation(Concatenation(Atom('E', 'e'), "
        "Optional(Atom('+', '-'))), PositiveKleene(Atom('0' - '9'))))), "
        "Optional(Atom('!', '#')))"sv);
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

static void dfa_simulation_float_literals()
{
    enum class Type : s32
    {
        Eof = s32(sigil::SpecialTokenType::Eof),
        Error = s32(sigil::SpecialTokenType::Error),
        IntLit,
        FloatLit,
    };
    sigil::Specification spec;
    spec.add_regex_token((s32)Type::IntLit, "IntLit", "\\d+"sv);
    spec.add_regex_token(
        (s32)Type::FloatLit,
        "FloatLit",
        R"END((\d+(\.\d*)?|\d*\.\d+)([eE][+-]?\d+)?)END"sv);

    auto either_grammar = sigil::Grammar::compile(spec);
    if (not either_grammar.isRight()) {
        debug_log("Error: ", either_grammar.left(), "\n");
        return;
    }

    auto grammar = std::move(either_grammar.release_right());

    using namespace sigil::dfa;

    expect_eq(simulate(grammar, "5"), SimulationResult::accept("IntLit"));
    expect_eq(simulate(grammar, "1."), SimulationResult::accept("FloatLit"));
    expect_eq(simulate(grammar, ".1"), SimulationResult::accept("FloatLit"));
    expect_eq(simulate(grammar, "1e2"), SimulationResult::accept("FloatLit"));
    expect_eq(simulate(grammar, "1e-2"), SimulationResult::accept("FloatLit"));
    expect_eq(simulate(grammar, "1e+2"), SimulationResult::accept("FloatLit"));
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
    char_set_tests();
    regex_parser_tests();
    dfa_simulation_tests_calculator();
    dfa_simulation_tests_conflict();
    dfa_simulation_float_literals();
    scanner_detect_eof_instead_of_error();
    user_controlled_token_values();
}

int main()
{
    sigil_tests();
    return EXIT_SUCCESS;
}