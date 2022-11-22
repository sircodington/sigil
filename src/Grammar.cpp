//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <algorithm>  // std::min

#include <sigil/Grammar.h>

#include <core/Arena.h>
#include <core/Formatting.h>
#include <core/Map.h>
#include <core/Set.h>

#include <sigil/Dfa.h>
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
            automaton.create_character_arc(start, end, exp->char_set());
        } break;

        case RegExp::Type::Alternative: {
            const auto exp = reinterpret_cast<const Alternative *>(regexp);

            auto left = create_regex_nfa(automaton, exp->left());
            drop_config(left);

            auto right = create_regex_nfa(automaton, exp->right());
            drop_config(right);

            automaton.create_epsilon_arc(start, left.start);
            automaton.create_epsilon_arc(start, right.start);
            automaton.create_epsilon_arc(left.end, end);
            automaton.create_epsilon_arc(right.end, end);
        } break;

        case RegExp::Type::Concatenation: {
            const auto exp = reinterpret_cast<const Concatenation *>(regexp);

            auto left = create_regex_nfa(automaton, exp->left());
            drop_config(left);

            auto right = create_regex_nfa(automaton, exp->right());
            drop_config(right);

            automaton.create_epsilon_arc(start, left.start);
            automaton.create_epsilon_arc(left.end, right.start);
            automaton.create_epsilon_arc(right.end, end);
        } break;

        case RegExp::Type::Kleene: {
            const auto exp = reinterpret_cast<const Kleene *>(regexp);

            auto wrapped = create_regex_nfa(automaton, exp->exp());
            drop_config(wrapped);

            automaton.create_epsilon_arc(start, wrapped.start);
            automaton.create_epsilon_arc(start, end);
            automaton.create_epsilon_arc(wrapped.end, end);
            automaton.create_epsilon_arc(end, start);
        } break;

        case RegExp::Type::PositiveKleene: {
            const auto exp = reinterpret_cast<const PositiveKleene *>(regexp);

            auto wrapped = create_regex_nfa(automaton, exp->exp());
            drop_config(wrapped);

            automaton.create_epsilon_arc(start, wrapped.start);
            automaton.create_epsilon_arc(wrapped.end, end);
            automaton.create_epsilon_arc(end, start);
        } break;

        case RegExp::Type::Optional: {
            const auto exp = reinterpret_cast<const Optional *>(regexp);

            auto wrapped = create_regex_nfa(automaton, exp->exp());
            drop_config(wrapped);

            automaton.create_epsilon_arc(start, wrapped.start);
            automaton.create_epsilon_arc(wrapped.end, end);
            automaton.create_epsilon_arc(start, end);
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
                automaton.create_character_arc(curr, next, CharSet(c));
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

        case Specification::TokenSpec::Type::Nfa: {
            sigil::nfa::Automaton automaton(arena);
            assert(token.build);
            token.build(automaton);
            if (automaton.arcs().is_empty() and automaton.states().is_empty())
                return Result::left("User code yielded an invalid automaton"sv);
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
    DfaState(Set<NfaState> nfa_states, dfa::State *dfa_state)
        : nfa_states(std::move(nfa_states))
        , dfa_state(dfa_state)
    {
    }
    Set<NfaState> nfa_states;
    dfa::State *dfa_state { nullptr };
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
                if (arc.is_character())
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
            if (arc.is_epsilon())
                return;
            if (not arc.char_set.contains(c))
                return;

            NfaState nfa_state { &nfa, arc.target };
            reachable.add(std::move(nfa_state));
        });
    }

    return reachable;
}

static Set<NfaState> dfa_start_state(
    dfa::Automaton &dfa,
    Map<Set<NfaState>, DfaState *> &mapping,
    const List<nfa::Automaton> &nfas)
{
    Set<NfaState> nfa_start_states;
    for (auto &nfa : nfas) {
        NfaState nfa_state { &nfa, nfa.start_state() };
        nfa_start_states.add(std::move(nfa_state));
    }
    return nfa_start_states;
}

static DfaState *create_or_get_dfa_state(
    core::Arena &arena,
    dfa::Automaton &dfa,
    Map<Set<NfaState>, DfaState *> &mapping,
    const Set<NfaState> &states)
{
    if (not mapping.contains(states)) {
        auto state = dfa.create_state();
        if (states.is_empty())
            state->type = dfa::State::Type::Error;
        auto new_state = arena.construct<DfaState>(states, state);
        mapping.add(states, new_state);
    }
    return mapping.get(states);
}

static s64 smallest_index_within(
    const List<nfa::Automaton> &nfas,
    const List<const nfa::Automaton *> &accepting)
{
    s64 smallest_index = std::numeric_limits<s64>::max();

    for (auto nfa : accepting) {
        s64 current_index = -1;
        for (s64 i = 0; i < nfas.size(); ++i) {
            if (&nfas[i] == nfa) {
                current_index = i;
                break;
            }
        }

        assert(current_index >= 0);

        smallest_index = std::min(smallest_index, current_index);
    }

    return smallest_index;
}

static void create_dfa(
    sigil::Grammar &grammar, const List<nfa::Automaton> &nfas)
{
    auto &dfa = grammar.dfa();
    Map<Set<NfaState>, DfaState *> mapping;
    List<DfaState *> dfa_state_queue;

    auto *dfa_start = create_or_get_dfa_state(
        grammar.arena(),
        dfa,
        mapping,
        reachable_by_epsilon(dfa_start_state(dfa, mapping, nfas)));
    dfa_start->dfa_state->start = true;
    dfa_state_queue.add(dfa_start);

    for (Index i = 0; i < dfa_state_queue.size(); ++i) {
        auto dfa_state = dfa_state_queue[i];

        for (auto c = CharSet::first; c <= CharSet::last; ++c) {
            auto reachable = reachable_by_epsilon(
                reachable_by_char(dfa_state->nfa_states, c));
            auto new_state = create_or_get_dfa_state(
                grammar.arena(), dfa, mapping, reachable);
            if (not dfa_state_queue.contains(new_state))
                dfa_state_queue.add(new_state);

            dfa::Arc *arc_between = nullptr;
            for (auto arc : dfa.arcs()) {
                if (arc->origin == dfa_state->dfa_state and
                    arc->target == new_state->dfa_state) {
                    arc_between = arc;
                    break;
                }
            }
            if (arc_between == nullptr) {
                arc_between = dfa.create_arc(
                    dfa_state->dfa_state, new_state->dfa_state, CharSet(c));
            }
            arc_between->char_set.set(c, true);
        }

        // @TODO: Optimize Set and Map
        // @TODO: Visualize automatons (maybe using graphvis)

        using Type = dfa::State::Type;
        if (Type::Error != dfa_state->dfa_state->type) {
            List<const nfa::Automaton *> accepting;
            for (const auto &nfa_state : dfa_state->nfa_states) {
                if (nfa_state.state->accepting)
                    accepting.add(nfa_state.nfa);
            }

            if (accepting.non_empty()) {
                dfa_state->dfa_state->type = Type::Accepting;
                dfa_state->dfa_state->token_index =
                    smallest_index_within(nfas, accepting);
            }
        }
    }
}

Either<StringView, Grammar> sigil::Grammar::compile(
    const sigil::Specification &specification)
{
    using Result = Either<StringView, Grammar>;
    Grammar grammar;

    List<nfa::Automaton> nfas(specification.tokens().size());
    for (const auto &token_spec : specification.tokens()) {
        auto either_automaton = create_nfa(grammar.arena(), token_spec);
        if (not either_automaton.isRight())
            return Result::left(std::move(either_automaton.release_left()));

        auto automaton = std::move(either_automaton.release_right());
        nfas.add(std::move(automaton));
        grammar.token_names().add(token_spec.name);
    }

    create_dfa(grammar, nfas);

    {
        auto &dfa = grammar.dfa();
        for (auto state : dfa.states()) {
            if (state->is_accepting()) {
                assert(state->token_index >= 0);
                auto &token = specification.tokens()[state->token_index];
                state->token_type = token.token_type;
            }
        }
    }

    return Result::right(std::move(grammar));
}

Grammar::Grammar()
    : m_dfa(m_arena)
{
}

}  // namespace sigil