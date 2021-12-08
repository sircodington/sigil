//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Either.h>
#include <core/String.h>
#include <core/StringView.h>

namespace sigil {

class RegExp;

class RegexParser
{
public:
    RegexParser() = default;
    void initialize(StringView input);
    Either<StringView, RegExp *> parse();

    static bool unescape(u8 &result, u8 &advance, const core::StringView &);
    static String escape(char c);

private:
    RegExp *parse_alternative();
    RegExp *parse_concatenation();
    RegExp *parse_postfix();
    uint64_t parse_atom();
    RegExp *parse_primary();

    // @TODO: This might be a bit nicer when using Option<char>
    [[nodiscard]] bool can_peek() const;
    char peek();
    char advance();

    s64 m_offset { -1 };
    StringView m_input;
};

}  // namespace sigil
