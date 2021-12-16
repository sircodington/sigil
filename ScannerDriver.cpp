//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "ScannerDriver.h"

namespace sigil {

void ScannerDriver::initialize(StringView file_path, StringView input)
{
    this->m_file_path = file_path;
    this->m_input = input;

    first_accepting = Position();
    last_accepting = Position();
    current = Position();

    m_has_next_token = false;
    m_scan_error = false;
    m_eof_token_returned = false;
    m_next_token = Token();
}

bool ScannerDriver::has_next()
{
    if (m_has_next_token)
        return true;
    if (m_scan_error)
        return false;

    get_next_token();
    return m_has_next_token or not m_eof_token_returned;
}

ScannerDriver::Token ScannerDriver::next()
{
    assert(has_next());

    if (not m_has_next_token) {
        m_eof_token_returned = true;
        Token eof_token {
            -2,
            {},  // @TODO: Maybe point to end of input?
            accepting_range(),
        };

        return eof_token;
    }

    auto token = m_next_token;
    m_has_next_token = false;
    m_next_token = Token();
    return token;
}

u8 ScannerDriver::get_char()
{
    assert(current.offset < m_input.size());
    const auto c = u8(m_input[current.offset++]);
    if (c == '\n') {
        ++current.line;
        current.column = 0;
    } else {
        ++current.column;
    }
    return c;
}

void ScannerDriver::get_next_token()
{
    State state = start_state();
    current.state = error_state();
    last_accepting = current;
    first_accepting = current;

    if (is_accepting_state(state)) {
        current.state = state;
        first_accepting = current;
        last_accepting = current;
    }
    while (not is_error_state(state) and current.offset < m_input.size()) {
        auto c = get_char();
        state = next_state(state, c);
        if (is_accepting_state(state)) {
            current.state = state;
            last_accepting = current;
        }
    }

    if (not is_error_state(last_accepting.state)) {
        StringView lexeme {
            m_input.data() + first_accepting.offset,
            s64(last_accepting.offset) - s64(first_accepting.offset),
        };

        Token token {
            accepting_token(last_accepting.state),
            lexeme,
            accepting_range(),
        };

        current = last_accepting;
        m_has_next_token = true;
        m_next_token = token;
    } else {
        if (is_error_state(current.state) and current.offset < m_input.size()) {
            Token error_token {
                -1,
                {},
                accepting_range(),
            };

            m_has_next_token = true;
            m_scan_error = true;
            m_next_token = error_token;
        }
    }
}

ScannerDriver::FileRange ScannerDriver::accepting_range() const
{
    return {
        m_file_path,
        { s64(first_accepting.line), s64(first_accepting.column) },
        { s64(last_accepting.line), s64(last_accepting.column) },
    };
}

}  // namespace sigil