//
// Copyright (c) 2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/StringView.h>
#include <core/Types.h>

#include <sigil/FileRange.h>

namespace sigil {

struct Token
{
    s32 type { s32(SpecialTokenType::Error) };
    StringView lexeme;
    FileRange range;
};

}  // namespace sigil