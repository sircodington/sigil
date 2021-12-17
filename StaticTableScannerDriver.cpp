//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "StaticTableScannerDriver.h"

namespace sigil {

StaticTableScannerDriver::StaticTableScannerDriver(const StaticTable &table)
    : m_start_state(table.start_state())
    , m_error_state(table.error_state())
    , m_transitions(table.transitions())
    , m_accepting(table.accepting())
{
}
StaticTable StaticTableScannerDriver::static_table() const
{
    return StaticTable(
        m_start_state, m_error_state, m_transitions, m_accepting);
}

}  // namespace sigil
