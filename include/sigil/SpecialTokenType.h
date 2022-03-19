//
// Copyright (c) 2022, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Types.h>

namespace sigil {

enum class SpecialTokenType : s32
{
    Eof = -2,
    Error = -1,
};

}  // namespace sigil