/*===============================================

    Forr Engine

    File : guid.hpp
    Role : GUID and type registry

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <random>
#include <string>
#include <sstream>

#include "attributes.hpp"

namespace fe {
    struct FORR_NODISCARD GUID {
        // TODO : provide GUID logic
    };

    // get an unique id for types
    struct TypeRegistry {
    public:
        using TypeID = size_t;

        template <typename T>
        FORR_NODISCARD static TypeID get_id() noexcept {
            static const TypeID id = S_COUNTER.fetch_add(1, std::memory_order_relaxed);
            return id;
        }

    private:
        inline static std::atomic<TypeID> S_COUNTER{};
    };
} // namespace fe

namespace std {
    template <>
    struct hash<fe::GUID> {
        std::size_t operator()(const fe::GUID& guid) const noexcept {
            // TODO : provide GUID logic
            return 0;
        }
    };
} // namespace std
