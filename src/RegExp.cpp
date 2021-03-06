//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/RegExp.h>

#include <cassert>

#include <core/Formatting.h>

namespace sigil {

RegExp::RegExp(RegExp::Type type)
    : m_type(type)
{
}

Atom::Atom(CharSet char_set)
    : RegExp(Type::Atom)
    , m_char_set(std::move(char_set))
{
}

Alternative::Alternative(RegExp *left, RegExp *right)
    : RegExp(Type::Alternative)
    , m_left(left)
    , m_right(right)
{
}

Concatenation::Concatenation(RegExp *left, RegExp *right)
    : RegExp(Type::Concatenation)
    , m_left(left)
    , m_right(right)
{
}

Kleene::Kleene(RegExp *exp)
    : RegExp(Type::Kleene)
    , m_exp(exp)
{
}

PositiveKleene::PositiveKleene(RegExp *exp)
    : RegExp(Type::PositiveKleene)
    , m_exp(exp)
{
}

Optional::Optional(RegExp *exp)
    : RegExp(Type::Optional)
    , m_exp(exp)
{
}

}  // namespace sigil

namespace core {

void Formatter<sigil::RegExp>::format(
    StringBuilder &b, const sigil::RegExp &value)
{
    switch (value.type()) {
        case sigil::RegExp::Type::Atom: {
            const auto &exp = reinterpret_cast<const sigil::Atom &>(value);
            Formatting::format_into(b, "Atom(", exp.char_set(), ")");
        } break;

        case sigil::RegExp::Type::Alternative: {
            const auto &exp =
                reinterpret_cast<const sigil::Alternative &>(value);
            Formatting::format_into(
                b, "Alternative(", *exp.left(), ", ", *exp.right(), ")");
        } break;

        case sigil::RegExp::Type::Concatenation: {
            const auto &exp =
                reinterpret_cast<const sigil::Concatenation &>(value);
            Formatting::format_into(
                b, "Concatenation(", *exp.left(), ", ", *exp.right(), ")");
        } break;

        case sigil::RegExp::Type::Kleene: {
            const auto &exp = reinterpret_cast<const sigil::Kleene &>(value);
            Formatting::format_into(b, "Kleene(", *exp.exp(), ")");
        } break;

        case sigil::RegExp::Type::PositiveKleene: {
            const auto &exp =
                reinterpret_cast<const sigil::PositiveKleene &>(value);
            Formatting::format_into(b, "PositiveKleene(", *exp.exp(), ")");
        } break;

        case sigil::RegExp::Type::Optional: {
            const auto &exp = reinterpret_cast<const sigil::Optional &>(value);
            Formatting::format_into(b, "Optional(", *exp.exp(), ")");
        } break;

        default: assert(false and "Unreachable");
    }
}

}  // namespace core