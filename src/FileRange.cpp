//
// Copyright (c) 2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include <sigil/FileRange.h>

#include <algorithm>

namespace sigil {

FileRange FileRange::merge(FileRange a, FileRange b)
{
    assert(a.file_path == b.file_path);
    FileRange result;
    result.file_path = a.file_path;
    result.first = std::min(a.first, b.first);
    result.end = std::max(a.end, b.end);
    return result;
}

}  // namespace sigil