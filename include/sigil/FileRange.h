//
// Copyright (c) 2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/StringView.h>

#include <sigil/FilePosition.h>

namespace sigil {

struct FileRange
{
    StringView file_path;
    FilePosition first;
    FilePosition end;  // exclusive

    static FileRange merge(FileRange, FileRange);
};

}  // namespace sigil