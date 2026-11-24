/*===============================================

    Forr Engine

    File : type_registry.hpp
    Role : type registry gets you an unique id for your type

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include "attributes.hpp"
#include "string.hpp"

namespace fe {
    using registered_type_id = size_t;

    namespace details {
        // the index must be in cpp file
        FORR_API registered_type_id resolve_type_id(fe::string_hash domain_hashed_name, fe::string_hash type_hashed_name);

        template <typename T>
        constexpr fe::string_hash get_hashed_name() noexcept {
#ifdef _MSC_VER
            return fe::make_hash(__FUNCSIG__);
#elif defined(__clang__) || defined(__GNUC__)
            return fe::make_hash(__PRETTY_FUNCTION__);
#endif
        }
    } // namespace details

    // @tparam Domain - this is like 'namespaces' to separate type indices in different systems.
    //                      the kind of the typename you put there doesn't affect anything,
    //                      the counting just will start over again,
    //                      so you can use even 'fe::type_registry<struct MyDomain>' syntax
    template <typename Domain>
    struct type_registry {
    public:
        using type_id = registered_type_id;

        // @tparam T - type, which's index you want to get in this Domain's scope
        template <typename T>
        FORR_NODISCARD static type_id get_id() {
            static const type_id id = static_cast<type_id>(details::resolve_type_id(details::get_hashed_name<Domain>(),
                                                                                    details::get_hashed_name<T>()));
            return id;
        }
    };
} // namespace fe
