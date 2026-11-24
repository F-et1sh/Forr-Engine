/*===============================================

    Forr Engine

    File : type_registry.hpp
    Role : type registry gets you an unique id for your type

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <random>
#include <string>
#include <sstream>

#include "attributes.hpp"

namespace fe {
    namespace details {
        // the index must be in cpp file
        FORR_API size_t next_type_id() noexcept;
    }

    // @tparam Domain - this is like 'namespaces' to separate type indices in different systems.
    //                      the kind of the typename you put there doesn't affect anything,
    //                      the counting just will start over again,
    //                      so you can use even 'fe::type_registry<struct MyDomain>' syntax
    template <typename Domain>
    struct type_registry {
    public:
        using type_id = size_t;

        // @tparam T - type, which's index you want to get in this Domain's scope
        template <typename T>
        FORR_NODISCARD static type_id get_id() noexcept {
            static const type_id id = static_cast<type_id>(details::next_type_id());
            return id;
        }
    };
} // namespace fe
