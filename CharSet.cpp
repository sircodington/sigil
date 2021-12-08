//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <algorithm>

#include "CharSet.h"

#include <core/Formatting.h>
#include <sigil/RegexParser.h>

namespace sigil {

CharSet::CharSet(u8 first, u8 last)
{
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        set(i, first <= u8(i) and u8(i) <= last);
    }
}

void CharSet::set(u8 i, bool value) { m_included[i] = value; }

void CharSet::set(u8 first, u8 last, bool value)
{
    const auto start = std::max<u16>(sigil::CharSet::first, first);
    const auto end = std::min<u16>(sigil::CharSet::last, last);
    for (auto i = start; i <= end; ++i) set(i, value);
}

void CharSet::negate()
{
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i)
        set(i, not contains(i));
}

}  // namespace sigil

namespace core {

void Formatter<sigil::CharSet>::format(
    StringBuilder &b, const sigil::CharSet &char_set)
{
    bool first = true;
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        const auto c = char(i);
        if (char_set.contains(c)) {
            if (not first)
                Formatting::format_into(b, ",");
            first = false;

            Formatting::format_into(b, "'", sigil::RegexParser::escape(c), "'");
        }
    }
}

}  // namespace core
