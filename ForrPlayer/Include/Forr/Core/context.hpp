/*===============================================

    Forr Engine

    File : context.hpp
    Role : unified context object that can contain any type

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <unordered_map>

#include "string.hpp"
#include "logging.hpp"
#include "type_registry.hpp"

namespace fe {
    // @brief context that can contain any type of object. important : it does not *store* your objects, you're responsible for their lifetime. it just gives you convenient access
    class context {
    private:
        using types = std::vector<void*>;

    public:
        // @brief add an object to the context
        // keep in mind that this context won't release your object - you're responsible for its lifetime
        //
        // @tparam T type of your object
        //
        // @param object lvalue of your object
        // @param tag optional tag to distinguish multiple objects of the same type ( 0 by default )
        template <typename T>
        void emplace(T& object, fe::string_hash tag = {}) {
            const type_registry<context>::type_id type_id = type_registry<context>::get_id<T>();

            auto it = m_objects.find(tag);
            if (it != m_objects.end()) {
                types& storage = it->second;

                if (type_id >= storage.size()) {
                    storage.resize(type_id + 1, nullptr);
                }
                else if (storage[type_id] != nullptr) {
                    fe::logging::error("Failed to emplace an object to the context. This type is already busy in this hash-scope\nHash-scope %llu\nType index was %llu",
                                       static_cast<uint64_t>(tag),
                                       static_cast<uint64_t>(type_id));
                    return;
                }
            }

            types& storage = m_objects[tag];
            if (type_id >= storage.size()) {
                storage.resize(type_id + 1, nullptr);
            }
            storage[type_id] = static_cast<void*>(&object);
        }

        // @brief get the object from the context
        //
        // @tparam T type of your object
        //
        // @param tag optional tag to distinguish multiple objects of the same type ( 0 by default )
        template <typename T>
        T& get(fe::string_hash tag = {}) {
            const type_registry<context>::type_id type_id = type_registry<context>::get_id<T>();

            auto it = m_objects.find(tag);
            if (it == m_objects.end()) {
                fe::logging::fatal("Failed to get an object from the context. No types were registered with this hash\nHash-scope %llu\nType index was %llu",
                                   static_cast<uint64_t>(tag),
                                   static_cast<uint64_t>(type_id));
            }
            else {
                types& storage = it->second;
                if (type_id >= storage.size() || storage[type_id] == nullptr) {
                    fe::logging::fatal("Failed to find a type. Type wasn't registered in this hash-scope\nHash-scope %llu\nType index was %llu",
                                       static_cast<uint64_t>(tag),
                                       static_cast<uint64_t>(type_id));
                }
                return *static_cast<T*>(storage[type_id]);
            }
        }

    private:
        // hashed name --> { T0, T1, T2, T3 }
        std::unordered_map<fe::string_hash, types> m_objects{};
    };
} // namespace fe
