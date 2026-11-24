/*===============================================

    Forr Engine

    File : type_registry.hpp
    Role : type registry gets you an unique id for your type

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Core/type_registry.hpp"

FORR_API size_t fe::details::next_type_id() noexcept {
    static std::atomic<size_t> S_COUNTER{ 0 };
    return S_COUNTER.fetch_add(1, std::memory_order_relaxed);
}
