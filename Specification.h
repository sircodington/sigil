//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Formatter.h>
#include <core/List.h>
#include <core/StringView.h>

namespace sigil {

class Specification
{
public:
    void add_literal_token(StringView token_name, StringView exact_string);
    void add_regex_token(StringView token_name, StringView regex);

    struct TokenSpec
    {
        enum class Type : u8
        {
            Invalid,
            Literal,
            Regex,
        };

        Type type { Type::Invalid };
        StringView name {};
        StringView pattern {};
    };

    [[nodiscard]] const List<TokenSpec> &tokens() const { return m_tokens; }

private:
    List<TokenSpec> m_tokens;
};

}  // namespace sigil

namespace core {

template<>
class Formatter<sigil::Specification::TokenSpec::Type>
{
public:
    static void format(StringBuilder &, sigil::Specification::TokenSpec::Type);
};

template<>
class Formatter<sigil::Specification::TokenSpec>
{
public:
    static void format(
        StringBuilder &, const sigil::Specification::TokenSpec &);
};

}  // namespace core