//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Grammar.h"

#include <core/Arena.h>
#include <core/Formatting.h>
#include <core/Map.h>
#include <core/Set.h>
#include <sigil/Nfa.h>
#include <sigil/RegExp.h>
#include <sigil/RegexParser.h>

namespace sigil {

nfa::State *start_state(nfa::Automaton &nfa)
{
    nfa::State *result = nullptr;
    for (auto &state : nfa.states()) {
        if (state->start) {
            assert(result == nullptr and "Multiple start states");
            result = state;
        }
    }
    return result;
}

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

static INfa create_regex_nfa(nfa::Automaton &automaton, const RegExp *regexp)
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

            auto left = create_regex_nfa(automaton, exp->left());
            drop_config(left);

            auto right = create_regex_nfa(automaton, exp->right());
            drop_config(right);

            epsilon_arc(automaton, start, left.start);
            epsilon_arc(automaton, start, right.start);
            epsilon_arc(automaton, left.end, end);
            epsilon_arc(automaton, right.end, end);
        } break;

        case RegExp::Type::Concatenation: {
            const auto exp = reinterpret_cast<const Concatenation *>(regexp);

            auto left = create_regex_nfa(automaton, exp->left());
            drop_config(left);

            auto right = create_regex_nfa(automaton, exp->right());
            drop_config(right);

            epsilon_arc(automaton, start, left.start);
            epsilon_arc(automaton, left.end, right.start);
            epsilon_arc(automaton, right.end, end);
        } break;

        case RegExp::Type::Kleene: {
            const auto exp = reinterpret_cast<const Kleene *>(regexp);

            auto wrapped = create_regex_nfa(automaton, exp->exp());
            drop_config(wrapped);

            epsilon_arc(automaton, start, wrapped.start);
            epsilon_arc(automaton, start, end);
            epsilon_arc(automaton, wrapped.end, end);
            epsilon_arc(automaton, end, start);
        } break;

        case RegExp::Type::PositiveKleene: {
            const auto exp = reinterpret_cast<const PositiveKleene *>(regexp);

            auto wrapped = create_regex_nfa(automaton, exp->exp());
            drop_config(wrapped);

            epsilon_arc(automaton, start, wrapped.start);
            epsilon_arc(automaton, wrapped.end, end);
            epsilon_arc(automaton, end, start);
        } break;

        case RegExp::Type::Optional: {
            const auto exp = reinterpret_cast<const Optional *>(regexp);

            auto wrapped = create_regex_nfa(automaton, exp->exp());
            drop_config(wrapped);

            epsilon_arc(automaton, start, wrapped.start);
            epsilon_arc(automaton, wrapped.end, end);
            epsilon_arc(automaton, start, end);
        } break;
    }

    return { start, end };
}

static Either<StringView, nfa::Automaton> create_nfa(
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
            create_regex_nfa(automaton, regex);
            return Result::right(std::move(automaton));
        }

        case Specification::TokenSpec::Type::Invalid:
        default: assert(false and "Unreachable"); return Result::left({});
    }
}

struct NfaState
{
    nfa::Automaton *nfa { nullptr };
    nfa::State *state { nullptr };

    bool operator==(const NfaState &other) const
    {
        return nfa == other.nfa and state == other.state;
    }
};

struct DfaState
{
    Set<NfaState> nfa_states;
    nfa::State *dfa_state;
};

template<typename Callback>
static void foreach_reachable(
    nfa::Automaton &automaton, nfa::State *state, Callback callback)
{
    for (auto arc : automaton.arcs()) {
        if (arc->origin == state)
            callback(*arc);
    }
}

static Set<NfaState> reachable_by_epsilon(const Set<NfaState> &states)
{
    Set<NfaState> result;
    for (auto state : states) result.add(state);

    bool modified = true;

    while (modified) {
        Set<NfaState> reachable;

        for (auto nfa_state : result) {
            auto &nfa = *nfa_state.nfa;

            foreach_reachable(nfa, nfa_state.state, [&](nfa::Arc &arc) {
                assert(arc.char_set.non_empty() ^ arc.epsilon);

                if (not arc.epsilon)
                    return;

                NfaState nfa_state { &nfa, arc.target };
                reachable.add(std::move(nfa_state));
            });
        }

        modified = false;
        for (auto elem : reachable) {
            if (not result.contains(elem)) {
                result.add(std::move(elem));
                modified = true;
            }
        }
    }

    return result;
}

static Set<NfaState> reachable_by_char(const Set<NfaState> &states, u8 c)
{
    Set<NfaState> reachable;

    for (auto &state : states) {
        auto &nfa = *state.nfa;

        foreach_reachable(nfa, state.state, [&](nfa::Arc &arc) {
            assert(arc.char_set.non_empty() ^ arc.epsilon);

            if (arc.epsilon)
                return;
            if (not arc.char_set.contains(c))
                return;

            NfaState nfa_state { &nfa, arc.target };
            reachable.add(std::move(nfa_state));
        });
    }

    return reachable;
}

static DfaState *dfa_start_state(
    nfa::Automaton &dfa,
    Map<Set<NfaState>, DfaState *> &mapping,
    const List<nfa::Automaton> &nfas)
{
    Set<NfaState> nfa_start_states;
    for (auto &nfa : nfas) {
        NfaState nfa_state { &nfa, start_state(nfa) };
        nfa_start_states.add(std::move(nfa_state));
    }

    auto nfa_states = reachable_by_epsilon(nfa_start_states);
    if (not mapping.contains(nfa_states)) {
        auto state = dfa.create_state();
        state->start = true;
        auto dfa_state = new DfaState { nfa_states, state };  // @TODO: Arena?
        mapping.add(nfa_states, dfa_state);
    }
    return mapping.get(nfa_states);
}

static nfa::Automaton create_dfa(
    core::Arena &arena, const List<nfa::Automaton> &nfas)
{
    nfa::Automaton dfa(arena);
    Map<Set<NfaState>, DfaState *> mapping;
    List<DfaState *> dfa_state_queue;

    auto *dfa_start = dfa_start_state(dfa, mapping, nfas);
    dfa_state_queue.add(dfa_start);

    for (Index i = 0; i < dfa_state_queue.size(); ++i) {
        auto dfa_state = dfa_state_queue[i];

        for (auto c = CharSet::first; c <= CharSet::last; ++c) {
            auto reachable = reachable_by_epsilon(
                reachable_by_char(dfa_state->nfa_states, c));

            if (not mapping.contains(reachable)) {
                auto state = dfa.create_state();
                auto new_state =
                    new DfaState { reachable, state };  // @TODO: Arena?
                dfa_state_queue.add(new_state);
                mapping.add(reachable, new_state);
            }
            auto new_state = mapping.get(reachable);

            nfa::Arc *arc_between = nullptr;
            for (auto arc : dfa.arcs()) {
                if (arc->origin == dfa_state->dfa_state and
                    arc->target == new_state->dfa_state) {
                    arc_between = arc;
                    break;
                }
            }
            if (arc_between == nullptr) {
                arc_between =
                    dfa.create_arc(dfa_state->dfa_state, new_state->dfa_state);
            }
            arc_between->char_set.set(c, true);
        }

        // @TODO: Different data structure for dfa?
        // @TODO: Flag explicit error state as error state (iff `reachable` is empty)
        // @TODO: Label accepting states (Remember which token they represent)
        // @TODO: Optimize allocations e.g. using Arena?
        // @TODO: Optimize Set and Map
        // @TODO: Unit-testing nfa?, dfa
        bool contains_accepting = false;
        for (const auto &nfa_state : dfa_state->nfa_states) {
            if (nfa_state.state->accepting) {
                contains_accepting = true;
                break;
            }
        }
        dfa_state->dfa_state->accepting = contains_accepting;
    }

    return dfa;
}

Either<StringView, Grammar> sigil::Grammar::compile(
    const sigil::Specification &specification)
{
    using Result = Either<StringView, Grammar>;
    core::Arena arena;

    List<nfa::Automaton> nfas(specification.tokens().size());
    for (Index i = 0; i < specification.tokens().size(); ++i) {
        auto either_automaton = create_nfa(arena, specification.tokens()[i]);
        if (not either_automaton.isRight())
            return Result::left(std::move(either_automaton.release_left()));

        auto automaton = std::move(either_automaton.release_right());

        //        // @TODO: Automaton can't be copied! Is this our Lists fault?
        //        core::StringBuilder builder;
        //        nfa::Automaton::format(builder, automaton);
        //        debug_log(builder.toString(), "\n");

        nfas.add(std::move(automaton));
    }

    auto dfa = create_dfa(arena, nfas);
    core::StringBuilder builder;
    nfa::Automaton::format(builder, dfa);
    debug_log(builder.toString(), "\n");

    Grammar grammar;
    return Either<StringView, Grammar>::right(std::move(grammar));
}

}  // namespace sigil