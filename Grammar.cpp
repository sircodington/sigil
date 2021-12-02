//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Grammar.h"

#include <core/Logging.h>
#include <sigil/Nfa.h>

namespace sigil {

static bool create_automaton(
    nfa::Automaton &automaton, const Specification::TokenSpec &token)
{
    switch (token.type) {
        case Specification::TokenSpec::Type::Literal: {
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
            return true;
        }

        case Specification::TokenSpec::Type::Regex: {
            assert(false and "Not implemented yet");
            return true;
        }

        case Specification::TokenSpec::Type::Invalid:
        default: assert(false and "Unreachable"); return false;
    }
}

Either<StringView, Grammar> sigil::Grammar::compile(
    const sigil::Specification &specification)
{
    using Result = Either<StringView, Grammar>;

    List<nfa::Automaton> nfas(specification.tokens().size());
    for (Index i = 0; i < specification.tokens().size(); ++i) {
        nfas.add(nfa::Automaton());
        if (not create_automaton(nfas[i], specification.tokens()[i])) {
            return Result::left("failed to create nfa for token");
        }

        // @TODO: Automaton can't be copied! Is this our Lists fault?

        nfa::Automaton::log(nfas[i]);
        core::Logging::log("\n");
    }

    Grammar grammar;
    return Either<StringView, Grammar>::right(std::move(grammar));
}

}  // namespace sigil