//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/StringView.h>

namespace sigil {

class ScannerDriver
{
public:
    ScannerDriver() = default;
    virtual ~ScannerDriver() = default;

    virtual void initialize(StringView file_path, StringView input);
    bool has_next();

    struct FilePosition
    {
        s64 line { -1 };
        s64 column { -1 };
    };
    struct FileRange
    {
        StringView file_path;
        FilePosition first;
        FilePosition end; // exclusive
    };
    struct Token
    {
        s64 type { -1 };  // -2 = eof-token, -1 = error token
        StringView lexeme;
        FileRange range;
    };
    Token next();

private:
    virtual u32 start_state() const = 0;
    virtual u32 error_state() const = 0;
    virtual s64 next_state(s64 state, s32 character) const = 0;
    virtual s64 is_accepting_state(s64 state) const = 0;
    virtual s64 is_error_state(s64 state) const = 0;
    virtual s64 accepting_token(s64 state) const = 0;

    s32 get_char();
    void get_next_token();

    StringView m_file_path;
    StringView m_input;

    struct Position
    {
        u64 offset { 0 };
        u64 line { 0 };
        u64 column { 0 };
        u32 state;
    };
    Position first_accepting;
    Position last_accepting;
    Position current;
    FileRange accepting_range() const;

    bool m_has_next_token { false };
    bool m_scan_error { false };
    bool m_eof_token_returned { false };
    Token m_next_token;
};

}  // namespace sigil
