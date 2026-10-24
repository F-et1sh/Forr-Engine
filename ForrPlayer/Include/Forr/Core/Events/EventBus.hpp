/*===============================================

    Forr Engine

    File : pointer.hpp
    Role : slot map

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <concepts>
#include <atomic>

#include "Forr/Core/attributes.hpp"
#include "Forr/Core/custom_allocators.hpp"

namespace fe {
    class EventBus {
    private:
        // get unique id for every event
        struct TypeRegistry {
            template <typename T>
            static size_t get_id() noexcept {
                static const size_t id = s_counter.fetch_add(1, std::memory_order_relaxed);
                return id;
            }

        private:
            inline static std::atomic<size_t> s_counter{};
        };
    };
} // namespace fe
