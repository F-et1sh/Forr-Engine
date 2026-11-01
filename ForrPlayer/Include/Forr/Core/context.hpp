/*===============================================

    Forr Engine

    File : context.hpp
    Role : contains 'fe::EngineContext', which is used to share systems with each other

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <unordered_map>

#include "logging.hpp"
#include "type_registry.hpp"

namespace fe {
    class EngineContext {
    public:
        template <typename T>
        void Register(T& system) {
            const TypeRegistry<EngineContext>::TypeID type_id = TypeRegistry<EngineContext>::get_id<T>();
            if (type_id >= m_types.size()) {
                m_types.resize(type_id + 1, nullptr);
            }
            if (m_types[type_id] != nullptr) {
                fe::logging::fatal("You can't register a type twice. If you want to store some parameter variables, create a propper class-context and register it.\nType index was %llu", type_id);
            }
            m_types[type_id] = static_cast<void*>(&system);
        }

        template <typename T>
        T& Get() {
            const TypeRegistry<EngineContext>::TypeID type_id = TypeRegistry<EngineContext>::get_id<T>();
            if (type_id >= m_types.size() || m_types[type_id] == nullptr) {
                fe::logging::fatal("Failed to find a type. Type wasn't registered.\nType index was %llu", type_id);
            }
            return *static_cast<T*>(m_types[type_id]);
        }

    private:
        // the context doesn't store the types
        std::vector<void*> m_types{};
    };
} // namespace fe
