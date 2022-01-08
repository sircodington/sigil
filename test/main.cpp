//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <core/Formatting.h>

#include <sigil/DfaSimulation.h>
#include <sigil/RegExp.h>
#include <sigil/RegexParser.h>

#define expect_eq(actual, expect)                                              \
  do {                                                                         \
    const auto x = (actual);                                                   \
    const auto y = (expect);                                                   \
    if (x != y) {                                                              \
      debug_log("Expected equality of values\n  ", #actual, "\n  ", x,         \
                "\n and\n  ", #expect, "\n  ", y, "\n\n");                     \
    }                                                                          \
  } while (0)

String parse_regex(const StringView &regex_pattern) {
  core::Arena arena;
  sigil::RegexParser parser(arena);
  parser.initialize(regex_pattern);
  auto either_exp = parser.parse();
  if (not either_exp.isRight())
    return core::Formatting::format("Parse error: ", either_exp.left());

  const auto regexp = either_exp.release_right();
  return core::Formatting::format(*regexp);
}

static void regex_parser_tests() {
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
  expect_eq(parse_regex("a*+?"), "Optional(PositiveKleene(Kleene(Atom('a'))))");

  expect_eq(parse_regex("ab|c"),
            "Alternative(Concatenation(Atom('a'), Atom('b')), Atom('c'))");
  expect_eq(parse_regex("a|bc"),
            "Alternative(Atom('a'), Concatenation(Atom('b'), Atom('c')))");

  expect_eq(parse_regex("(a)"), "Atom('a')");
  expect_eq(parse_regex("a(b|c)"),
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

static void dfa_simulation_tests_calculator() {
  sigil::Specification spec;
  spec.add_literal_token("Plus", "+");
  spec.add_literal_token("Star", "*");
  spec.add_literal_token("OpenParenthesis", "(");
  spec.add_literal_token("CloseParenthesis", ")");
  spec.add_regex_token("Literal", "[0-9]+");
  spec.add_regex_token("Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
  spec.add_regex_token("Whitespace", "[ \\n\\r\\t]+");

  auto either_grammar = sigil::Grammar::compile(spec);
  if (not either_grammar.isRight()) {
    debug_log("Error: ", either_grammar.left(), "\n");
    return;
  }

  auto grammar = std::move(either_grammar.release_right());

  using namespace sigil::dfa;

  expect_eq(simulate(grammar, "+"), SimulationResult::accept("Plus"));
  expect_eq(simulate(grammar, "*"), SimulationResult::accept("Star"));
  expect_eq(simulate(grammar, "("),
            SimulationResult::accept("OpenParenthesis"));
  expect_eq(simulate(grammar, ")"),
            SimulationResult::accept("CloseParenthesis"));

  expect_eq(simulate(grammar, " "), SimulationResult::accept("Whitespace"));
  expect_eq(simulate(grammar, "  "), SimulationResult::accept("Whitespace"));
  expect_eq(simulate(grammar, "\n"), SimulationResult::accept("Whitespace"));
  expect_eq(simulate(grammar, "\n\r"), SimulationResult::accept("Whitespace"));
  expect_eq(simulate(grammar, "\r\n"), SimulationResult::accept("Whitespace"));
  expect_eq(simulate(grammar, "\t"), SimulationResult::accept("Whitespace"));

  expect_eq(simulate(grammar, "0"), SimulationResult::accept("Literal"));
  expect_eq(simulate(grammar, "1"), SimulationResult::accept("Literal"));
  expect_eq(simulate(grammar, "10"), SimulationResult::accept("Literal"));
  expect_eq(simulate(grammar, "9999"), SimulationResult::accept("Literal"));
  expect_eq(simulate(grammar, "12345"), SimulationResult::accept("Literal"));

  expect_eq(simulate(grammar, "if"), SimulationResult::accept("Identifier"));
  expect_eq(simulate(grammar, "ifx"), SimulationResult::accept("Identifier"));
  expect_eq(simulate(grammar, "abc"), SimulationResult::accept("Identifier"));
  expect_eq(simulate(grammar, "my_list"),
            SimulationResult::accept("Identifier"));
  expect_eq(simulate(grammar, "Test_3"),
            SimulationResult::accept("Identifier"));
}

static void dfa_simulation_tests_conflict() {
  sigil::Specification spec;
  spec.add_literal_token("KwIf", "if");
  spec.add_regex_token("Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");

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
  expect_eq(simulate(grammar, "my_list"),
            SimulationResult::accept("Identifier"));
  expect_eq(simulate(grammar, "Test_3"),
            SimulationResult::accept("Identifier"));
}

void sigil_tests() {
  regex_parser_tests();
  dfa_simulation_tests_calculator();
  dfa_simulation_tests_conflict();
}

int main() {
  sigil_tests();
  return EXIT_SUCCESS;
}