//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Arena.h>
#include <core/Either.h>
#include <core/String.h>
#include <core/StringView.h>

namespace sigil {

class RegExp;

class RegexParser
{
public:
    explicit RegexParser(core::Arena &arena);
    void initialize(StringView input);
    Either<StringView, RegExp *> parse();

    static bool unescape(u8 &result, u8 &advance, const core::StringView &);
    static String escape(u8 c);

private:
    template<typename T, typename... Args>
    inline RegExp *create_reg_exp(Args &&...args)
    {
        return m_arena.construct<T>(std::forward<Args>(args)...);
    }

    RegExp *parse_alternative();
    RegExp *parse_concatenation();
    RegExp *parse_postfix();
    uint64_t parse_atom();
    RegExp *parse_primary();

    // @TODO: This might be a bit nicer when using Option<char>
    [[nodiscard]] bool can_peek() const;
    char peek();
    char advance();

    core::Arena &m_arena;
    s64 m_offset { -1 };
    StringView m_input;
};

}  // namespace sigil
