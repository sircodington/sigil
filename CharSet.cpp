//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "CharSet.h"

#include <core/Logging.h>

namespace sigil {

CharSet::CharSet(u8 first, u8 last)
{
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        m_included[i] = first <= u8(i) and u8(i) <= last;
    }
}

}  // namespace sigil

namespace core {

void Logger<const sigil::CharSet &>::log(const sigil::CharSet &char_set)
{
    bool first = true;
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        const auto c = char(i);
        if (char_set.contains(c)) {
            if (not first)
                Logging::log(",");
            first = false;

            if (' ' <= c and c <= '~') {
                Logging::log("'", c, "'");
            } else {
                Logging::log(i);
            }
        }
    }
}

}  // namespace core