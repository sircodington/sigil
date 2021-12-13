//
// Copyright (c) 2021, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <core/Formatter.h>
#include <sigil/Grammar.h>

namespace sigil::dfa {

class SimulationResult
{
public:
    enum class Type : u8
    {
        Any,
        Error,
        Accept,
    };

    static SimulationResult any();
    static SimulationResult error();
    static SimulationResult accept(StringView token_name);

    bool operator==(const SimulationResult &other) const
    {
        return m_type == other.m_type and m_token_name == other.m_token_name;
    }
    bool operator!=(const SimulationResult &other) const
    {
        return not(*this == other);
    }

    [[nodiscard]] Type type() const { return m_type; }
    [[nodiscard]] StringView token_name() const { return m_token_name; }

private:
    SimulationResult(Type type, StringView token_name);

    Type m_type { Type::Any };
    StringView m_token_name;
};

SimulationResult simulate(const sigil::Grammar &, StringView source);

}  // namespace sigil::dfa

namespace core {

template<>
class Formatter<sigil::dfa::SimulationResult>
{
public:
    static void format(StringBuilder &, const sigil::dfa::SimulationResult &);
};

}  // namespace core
