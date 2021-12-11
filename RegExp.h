//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Formatter.h>
#include <core/Types.h>
#include <sigil/CharSet.h>

namespace sigil {

class RegExp
{
public:
    enum class Type : u8
    {
        Invalid,
        Atom,
        Alternative,
        Concatenation,
        Kleene,
        PositiveKleene,
        Optional,
    };

    [[nodiscard]] Type type() const { return m_type; }

protected:
    explicit RegExp(Type type);

private:
    Type m_type { Type::Invalid };
};

class Atom final : public RegExp
{
public:
    explicit Atom(CharSet);

    [[nodiscard]] const CharSet &char_set() const { return m_char_set; }

private:
    CharSet m_char_set;
};

class Alternative final : public RegExp
{
public:
    Alternative(RegExp *left, RegExp *right);

    [[nodiscard]] const RegExp *left() const { return m_left; }
    [[nodiscard]] const RegExp *right() const { return m_right; }

private:
    RegExp *m_left { nullptr };
    RegExp *m_right { nullptr };
};

class Concatenation final : public RegExp
{
public:
    Concatenation(RegExp *left, RegExp *right);

    [[nodiscard]] const RegExp *left() const { return m_left; }
    [[nodiscard]] const RegExp *right() const { return m_right; }

private:
    RegExp *m_left { nullptr };
    RegExp *m_right { nullptr };
};

class Kleene final : public RegExp
{
public:
    explicit Kleene(RegExp *exp);

    [[nodiscard]] const RegExp *exp() const { return m_exp; }

private:
    RegExp *m_exp { nullptr };
};

class PositiveKleene final : public RegExp
{
public:
    explicit PositiveKleene(RegExp *exp);

    [[nodiscard]] const RegExp *exp() const { return m_exp; }

private:
    RegExp *m_exp { nullptr };
};

class Optional final : public RegExp
{
public:
    explicit Optional(RegExp *exp);

    [[nodiscard]] const RegExp *exp() const { return m_exp; }

private:
    RegExp *m_exp { nullptr };
};

}  // namespace sigil

namespace core {

template<>
class Formatter<sigil::RegExp>
{
public:
    static void format(StringBuilder &, const sigil::RegExp &);
};

}  // namespace core
