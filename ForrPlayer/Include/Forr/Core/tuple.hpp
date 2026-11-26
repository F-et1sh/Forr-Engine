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

    template <typename... Ts>
    struct tuple_remove_last_element;

    template <typename T, typename... Ts>
    struct tuple_remove_last_element<T, Ts...> {
        using type = decltype(std::tuple_cat(std::declval<std::tuple<T>>(),
                                             std::declval<typename tuple_remove_last_element<Ts...>::type>()));
    };

    template <typename Last>
    struct tuple_remove_last_element<Last> {
        using type = std::tuple<>;
    };

    template <typename... Ts>
    using tuple_remove_last_element_t = tuple_remove_last_element<Ts...>::type;
} // namespace fe
