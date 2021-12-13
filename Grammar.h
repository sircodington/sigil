//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Either.h>
#include <core/StringView.h>
#include <sigil/Specification.h>

namespace sigil {

class Grammar
{
public:
    Grammar(const Grammar &) = delete;
    Grammar(Grammar &&) = default;
    Grammar &operator=(const Grammar &) = delete;
    Grammar &operator=(Grammar &&) = default;

    static Either<StringView, Grammar> compile(const Specification &);
};

}  // namespace sigil
