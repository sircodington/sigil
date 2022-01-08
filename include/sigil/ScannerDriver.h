//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/RingBuffer.h>
#include <core/StringView.h>

#include <sigil/Types.h>

namespace sigil {

// @TODO: generate scanner driver implementation
class ScannerDriver
{
public:
    ScannerDriver() = default;
    virtual ~ScannerDriver() = default;

    virtual void initialize(StringView file_path, StringView input);

    // @TODO: Extract those structs
    struct FilePosition
    {
        s64 line { -1 };
        s64 column { -1 };
    };
    struct FileRange
    {
        StringView file_path;
        FilePosition first;
        FilePosition end;  // exclusive
    };
    enum class SpecialTokenType : s32
    {
        Eof = -2,
        Error = -1,
    };
    struct Token
    {
        s32 type { s32(SpecialTokenType::Error) };
        StringView lexeme;
        FileRange range;
    };

    inline bool can_lookahead(Index offset = 0)
    {
        return require_offset(offset);
    }
    inline const Token &lookahead(Index offset = 0)
    {
        assert(can_lookahead(offset));
        return m_lookahead.lookahead(offset);
    }
    inline bool can_consume(Size count = 1)
    {
        return require_offset(count - 1);
    }
    inline Token consume(Size count = 1)
    {
        assert(can_consume(count));
        for (Size i = 0; i < count; ++i) {
            auto token = m_lookahead.consume();
            if (i == count - 1)
                return token;
        }

        assert(false and "Unreachable");
        return {};
    }

    // @FIXME: Switching to the lookahead api makes the scanner slower
    bool has_next();
    Token next();

private:
    constexpr static Size Lookahead { 64 };
    inline bool require_offset(Index offset)
    {
        assert(offset <= Lookahead);
        if (m_lookahead.size() <= offset) {
            while (not m_lookahead.full() and has_next()) {
                m_lookahead.write(next());
            }
        }
        return offset < m_lookahead.size();
    }

    [[nodiscard]] virtual State start_state() const = 0;
    [[nodiscard]] virtual State error_state() const = 0;
    [[nodiscard]] virtual State next_state(State state, u8 c) const = 0;
    [[nodiscard]] virtual bool is_accepting_state(State state) const = 0;
    [[nodiscard]] virtual bool is_error_state(State state) const = 0;
    [[nodiscard]] virtual TokenType accepting_token(State state) const = 0;

    u8 get_char();
    void get_next_token();

    StringView m_file_path;
    StringView m_input;

    struct Position
    {
        u64 offset { 0 };
        u64 line { 0 };
        u64 column { 0 };
        State state;
    };
    Position first_accepting;
    Position last_accepting;
    Position current;
    FileRange accepting_range() const;

    bool m_has_next_token { false };
    bool m_scan_error { false };
    bool m_eof_token_returned { false };
    Token m_next_token;

    RingBuffer<Token, Lookahead> m_lookahead;
};

}  // namespace sigil
