//
// Copyright (c) 2021-2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Arena.h>
#include <core/Either.h>
#include <core/String.h>
#include <core/StringView.h>

#include <sigil/CharSet.h>

namespace sigil {

class RegExp;

class RegexParser
{
public:
    explicit RegexParser(core::Arena &arena);
    void initialize(StringView input);
    Either<StringView, RegExp *> parse();

private:
    template<typename T, typename... Args>
    inline RegExp *create_reg_exp(Args &&...args)
    {
        return m_arena.construct<T>(std::forward<Args>(args)...);
    }

    RegExp *parse_alternative();
    RegExp *parse_concatenation();
    RegExp *parse_postfix();
    RegExp *parse_atom();

    RegExp *parse_nested_atom();
    RegExp *parse_class_atom();
    RegExp *parse_top_level_atom();

    /// Parse single top-level atom, which may be \d or .
    CharSet parse_class_chars();
    /// Parse single atom in a class, which may be a range
    CharSet parse_top_level_chars();

    // @TODO: This might be a bit nicer when using Option<char>
    [[nodiscard]] bool can_peek() const;
    char peek();
    char advance();

    core::Arena &m_arena;
    s64 m_offset { -1 };
    StringView m_input;
};

}  // namespace sigil
