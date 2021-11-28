//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <limits>

#include <core/Logger.h>
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

    [[nodiscard]] bool contains(u8 c) const { return m_included[c]; }

private:
    constexpr static auto Size = std::numeric_limits<u8>::max() + 1;
    bool m_included[Size] {};
};

}  // namespace sigil

namespace core {

template<>
class Logger<const sigil::CharSet &>
{
public:
    static void log(const sigil::CharSet &);
};

}  // namespace core
