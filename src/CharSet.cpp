//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/CharSet.h>

#include <algorithm>

#include <core/Formatting.h>

#include <sigil/RegexParser.h>

namespace sigil {

CharSet::CharSet(u8 first, u8 last)
{
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        set(i, first <= u8(i) and u8(i) <= last);
    }
}

bool CharSet::is_empty() const
{
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        if (contains(i))
            return false;
    }
    return true;
}

void CharSet::set(u8 i, bool value) { m_included.set(i, value); }

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

CharSet CharSet::operator~() const
{
    CharSet result = *this;
    result.negate();
    return result;
}

template<typename BiPredicate>
inline static CharSet binary_operation(
    CharSet a, CharSet b, BiPredicate predicate)
{
    CharSet result;
    for (auto i = CharSet::first; i <= CharSet::last; ++i) {
        result.set(i, predicate(a.contains(i), b.contains(i)));
    }
    return result;
}

CharSet CharSet::operator|(CharSet other) const
{
    return binary_operation(
        *this, other, [](bool a, bool b) { return a or b; });
}

CharSet CharSet::operator&(CharSet other) const
{
    return binary_operation(
        *this, other, [](bool a, bool b) { return a and b; });
}


CharSet CharSet::operator/(CharSet other) const
{
    return binary_operation(
        *this, other, [](bool a, bool b) { return a and not b; });
}

}  // namespace sigil

namespace core {

void Formatter<sigil::CharSet>::format(
    StringBuilder &b, const sigil::CharSet &char_set)
{
    struct Range
    {
        bool empty { true };
        u8 first { 0 };
        u8 last { 0 };
    };

    bool first = true;
    auto emit = [&](const Range &range) {
        if (range.empty)
            return;
        if (not first)
            Formatting::format_into(b, ", ");
        first = false;

        Formatting::format_into(
            b, "'", sigil::RegexParser::escape(range.first), "'");
        if (range.first != range.last) {
            Formatting::format_into(
                b, " - '", sigil::RegexParser::escape(range.last), "'");
        }
    };

    Range current;
    for (auto i = sigil::CharSet::first; i <= sigil::CharSet::last; ++i) {
        const auto c = u8(i);
        if (char_set.contains(c)) {
            if (current.last + 1 == c) {
                // extend range
                current.empty = false;
                current.last = c;
            } else {
                emit(current);
                current = Range();
                current.empty = false;
                current.first = c;
                current.last = c;
            }
        }
    }
    emit(current);
}

}  // namespace core
