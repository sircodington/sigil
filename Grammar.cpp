//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "Grammar.h"

namespace sigil {

Either<StringView, Grammar> sigil::Grammar::compile(
    const sigil::Specification &)
{
    return Either<StringView, Grammar>::left("not implemented");
}

}  // namespace sigil