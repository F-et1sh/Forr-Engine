/*===============================================

    Forr Engine

    File : type_registry.hpp
    Role : type registry gets you an unique id for your type

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Core/type_registry.hpp"

#include <unordered_map>
#include <mutex>

fe::registered_type_id fe::details::resolve_type_id(fe::string_hash domain_hashed_name, fe::string_hash type_hashed_name) {
    static std::mutex mutex{};
    // domain hashed name --> { type hashed name --> type id }
    static std::unordered_map<fe::string_hash, std::unordered_map<fe::string_hash, fe::registered_type_id>> registry{};

    std::lock_guard<std::mutex> lock{ mutex };
    auto&                       domain_map = registry[domain_hashed_name];

    auto [it, inserted] = domain_map.try_emplace(type_hashed_name, domain_map.size());
    return it->second;
}
