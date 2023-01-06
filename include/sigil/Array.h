//
// Copyright (c) 2023, Jan Sladek <keddelzz@web.de>
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include <cassert>

#include <core/ListView.h>
#include <core/Types.h>

namespace sigil {

template<typename T>
class Array
{
public:
    Array();

    template<Size N>
    static Array<T> static_array(const T (&data)[N]);

    static Array<T> string_literal(const char *, Size);

    static Array<T> list_view(core::ListView<T>);

    [[nodiscard]] const void *data() const { return m_data; }
    [[nodiscard]] Size size() const { return m_size; }
    [[nodiscard]] bool is_empty() const { return size() == 0; }
    [[nodiscard]] bool non_empty() const { return not is_empty(); }
    [[nodiscard]] bool in_bounds(Index index) const { return index <= size(); }

    const T &operator[](Index) const;

private:
    Array(const void *, Size);

    const void *m_data { nullptr };
    Size m_size { 0 };
};

template<typename T>
Array<T>::Array(const void *data, Size size)
    : m_data(data)
    , m_size(size)
{
}

template<typename T>
Array<T>::Array()
    : Array(nullptr, 0)
{
}

template<typename T>
template<Size N>
Array<T> Array<T>::static_array(const T (&data)[N])
{
    return Array<T>(&data, N);
}

template<typename T>
Array<T> Array<T>::string_literal(const char *data, Size size)
{
    return Array<T>(data, size);
}

template<typename T>
Array<T> Array<T>::list_view(core::ListView<T> list)
{
    if (list.is_empty())
        return Array<T>();
    return Array<T>(&list[0], list.size());
}

template<typename T>
const T &Array<T>::operator[](Index index) const
{
    assert(in_bounds(index));
    const auto array = reinterpret_cast<const T *>(m_data);
    return array[index];
}

}  // namespace sigil
