//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "CharSet.h"

#include <core/Formatting.h>
#include <sigil/RegexParser.h>

namespace sigil {

CharSet::CharSet(u8 first, u8 last)
{
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        m_included[i] = first <= u8(i) and u8(i) <= last;
    }
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
