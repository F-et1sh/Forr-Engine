/*===============================================

    Forr Engine

    File : tuple.hpp
    Role : provides helpers to work with std::tuple

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <tuple>

namespace fe {
    template <typename T>
    struct is_tuple : std::false_type {};

    template <typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};
} // namespace fe
