//
// Copyright (c) 2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Types.h>

namespace sigil {

struct FilePosition
{
    s64 line { -1 };
    s64 column { -1 };
    [[nodiscard]] constexpr bool operator<(const FilePosition &other) const
    {
        if (line != other.line)
            return line < other.line;
        return column < other.column;
    }
};

}  // namespace sigil
