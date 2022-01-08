//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <limits>

#include <core/BitField.h>
#include <core/Formatter.h>
#include <core/Types.h>

namespace sigil {

// @TODO: Also support non-ascii characters

class CharSet
{
public:
    CharSet() = default;
    explicit CharSet(u8 c)
        : CharSet(c, c)
    {
    }
    CharSet(u8 first, u8 last);

    constexpr static u16 first = std::numeric_limits<u8>::min();
    constexpr static u16 last = std::numeric_limits<u8>::max();

    [[nodiscard]] bool contains(u8 c) const { return m_included.get(c); }
    bool is_empty() const;
    bool non_empty() const { return not is_empty(); }

    void set(u8, bool);
    void set(u8 first, u8 last, bool);
    void negate();

private:
    constexpr static auto Size = std::numeric_limits<u8>::max() + 1;
    BitField<Size> m_included;
};

}  // namespace sigil

namespace core {

template<>
class Formatter<sigil::CharSet>
{
public:
    static void format(StringBuilder &, const sigil::CharSet &);
};

}  // namespace core
