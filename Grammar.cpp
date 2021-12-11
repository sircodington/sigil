//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Grammar.h"

#include <core/Arena.h>
#include <core/Formatting.h>
#include <sigil/Nfa.h>
#include <sigil/RegExp.h>
#include <sigil/RegexParser.h>

namespace sigil {

struct INfa
{
    nfa::State *start { nullptr };
    nfa::State *end { nullptr };
};

static void drop_config(INfa &nfa)
{
    nfa.start->start = false;
    nfa.end->accepting = false;
}

static void epsilon_arc(nfa::Automaton &automaton, nfa::State *a, nfa::State *b)
{
    auto arc = automaton.create_arc(a, b);
    arc->epsilon = true;
}

static INfa create_regex_automaton(
    nfa::Automaton &automaton, const RegExp *regexp)
{
    auto start = automaton.create_state();
    start->start = true;
    auto end = automaton.create_state();
    end->accepting = true;

    switch (regexp->type()) {
        case RegExp::Type::Invalid: assert(false and "Unreachable"); return {};

        case RegExp::Type::Atom: {
            const auto exp = reinterpret_cast<const Atom *>(regexp);
            automaton.create_arc(start, end)->char_set = exp->char_set();
        } break;

        case RegExp::Type::Alternative: {
            const auto exp = reinterpret_cast<const Alternative *>(regexp);

            auto left = create_regex_automaton(automaton, exp->left());
            drop_config(left);

            auto right = create_regex_automaton(automaton, exp->right());
            drop_config(right);

            epsilon_arc(automaton, start, left.start);
            epsilon_arc(automaton, start, right.start);
            epsilon_arc(automaton, left.end, end);
            epsilon_arc(automaton, right.end, end);
        } break;

        case RegExp::Type::Concatenation: {
            const auto exp = reinterpret_cast<const Concatenation *>(regexp);

            auto left = create_regex_automaton(automaton, exp->left());
            drop_config(left);

            auto right = create_regex_automaton(automaton, exp->right());
            drop_config(right);

            epsilon_arc(automaton, start, left.start);
            epsilon_arc(automaton, left.end, right.start);
            epsilon_arc(automaton, right.end, end);
        } break;

        case RegExp::Type::Kleene: {
            const auto exp = reinterpret_cast<const Kleene *>(regexp);

            auto wrapped = create_regex_automaton(automaton, exp->exp());
            drop_config(wrapped);

            epsilon_arc(automaton, start, wrapped.start);
            epsilon_arc(automaton, start, end);
            epsilon_arc(automaton, wrapped.end, end);
            epsilon_arc(automaton, end, start);
        } break;

        case RegExp::Type::PositiveKleene: {
            const auto exp = reinterpret_cast<const PositiveKleene *>(regexp);

            auto wrapped = create_regex_automaton(automaton, exp->exp());
            drop_config(wrapped);

            epsilon_arc(automaton, start, wrapped.start);
            epsilon_arc(automaton, wrapped.end, end);
            epsilon_arc(automaton, end, start);
        } break;

        case RegExp::Type::Optional: {
            const auto exp = reinterpret_cast<const Optional *>(regexp);

            auto wrapped = create_regex_automaton(automaton, exp->exp());
            drop_config(wrapped);

            epsilon_arc(automaton, start, wrapped.start);
            epsilon_arc(automaton, wrapped.end, end);
            epsilon_arc(automaton, start, end);
        } break;
    }

    return { start, end };
}

static Either<StringView, nfa::Automaton> create_automaton(
    core::Arena &arena, const Specification::TokenSpec &token)
{
    using Result = Either<StringView, nfa::Automaton>;

    switch (token.type) {
        case Specification::TokenSpec::Type::Literal: {
            sigil::nfa::Automaton automaton(arena);
            auto curr = automaton.create_state();
            curr->start = true;

            // @TODO: Introduce StringView::begin and StringView::end
            for (Index i = 0; i < token.pattern.size(); ++i) {
                const auto c = token.pattern[i];

                auto next = automaton.create_state();
                auto arc = automaton.create_arc(curr, next);
                arc->char_set = CharSet(c);
                curr = next;
            }

            curr->accepting = true;
            return Result::right(std::move(automaton));
        }

        case Specification::TokenSpec::Type::Regex: {
            sigil::nfa::Automaton automaton(arena);
            sigil::RegexParser parser(arena);
            parser.initialize(token.pattern);

            auto either_regex = parser.parse();
            if (not either_regex.isRight())
                return Result::left(std::move(either_regex.release_left()));

            auto regex = either_regex.right();
            create_regex_automaton(automaton, regex);
            return Result::right(std::move(automaton));
        }

        case Specification::TokenSpec::Type::Invalid:
        default: assert(false and "Unreachable"); return Result::left({});
    }
}

Either<StringView, Grammar> sigil::Grammar::compile(
    const sigil::Specification &specification)
{
    using Result = Either<StringView, Grammar>;
    core::Arena arena;

    List<nfa::Automaton> nfas(specification.tokens().size());
    for (Index i = 0; i < specification.tokens().size(); ++i) {
        auto either_automaton =
            create_automaton(arena, specification.tokens()[i]);
        if (not either_automaton.isRight())
            return Result::left(std::move(either_automaton.release_left()));

        auto automaton = std::move(either_automaton.release_right());

        // @TODO: Automaton can't be copied! Is this our Lists fault?
        core::StringBuilder builder;
        nfa::Automaton::format(builder, automaton);
        debug_log(builder.toString(), "\n");

        nfas.add(std::move(automaton));
    }

    Grammar grammar;
    return Either<StringView, Grammar>::right(std::move(grammar));
}

}  // namespace sigil