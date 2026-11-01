/*===============================================

    Forr Engine

    File : type_registry.hpp
    Role : type registry

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <random>
#include <string>
#include <sstream>

#include "attributes.hpp"

namespace fe {
    // @tparam Category - this is like 'namespaces' to separate type indices in different systems.
    //                      the kind of the typename you put there doesn't affect anything, 
    //                      the counting just will start over again,
    //                      so you can use even 'fe::TypeRegistry<struct MyCategory>' syntax
    template <typename Category>
    struct TypeRegistry {
    public:
        using TypeID = size_t;

        // @tparam T - type, which's index you want to get in this Category's scope
        template <typename T>
        FORR_NODISCARD static TypeID get_id() noexcept {
            static const TypeID id = S_COUNTER.fetch_add(1, std::memory_order_relaxed);
            return id;
        }

    private:
        inline static std::atomic<TypeID> S_COUNTER{};
    };
} // namespace fe
