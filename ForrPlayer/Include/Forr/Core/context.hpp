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
    // @brief context that can contain any type of object; important : it does not *store* your objects, you're responsible for their lifetime. it just gives you convenient access
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
        //
        // @returns 'true' if everything is alright, 'false' if emplacement is failed
        template <typename T>
        bool emplace(T& object, fe::string_hash tag = {}) {
            const type_registry<context>::type_id type_id = type_registry<context>::get_id<T>();

            auto it = m_objects.find(tag);
            if (it != m_objects.end()) {
                types& storage = it->second;

                if (type_id >= storage.size()) {
                    storage.resize(type_id + 1, nullptr);
                }
                else if (storage[type_id] != nullptr) {
                    // TODO : think about removing this completely. this comments may annoy, while using 'fe::context::emplace_or_replace()'
                    //
                    //fe::logging::error("Failed to emplace an object to the context. This type is already busy in this Tag\nTag %llu\nType index was %llu",
                    //                   static_cast<uint64_t>(tag),
                    //                   static_cast<uint64_t>(type_id));
                    return false;
                }
            }

            types& storage = m_objects[tag];
            if (type_id >= storage.size()) {
                storage.resize(type_id + 1, nullptr);
            }
            storage[type_id] = static_cast<void*>(&object);

            return true;
        }

        // @brief replace an object in the context
        // keep in mind that this context won't release your object - you're responsible for its lifetime
        //
        // @tparam T type of your object
        //
        // @param object lvalue of your new object
        // @param tag optional tag to distinguish multiple objects of the same type ( 0 by default )
        //
        // @returns 'true' if everything is alright, 'false' if replacement is failed
        template <typename T>
        bool replace(T& object, fe::string_hash tag = {}) {
            const type_registry<context>::type_id type_id = type_registry<context>::get_id<T>();

            auto it = m_objects.find(tag);
            if (it != m_objects.end()) {
                types& storage = it->second;

                if (type_id >= storage.size()) {
                    storage.resize(type_id + 1, nullptr);
                }

                storage[type_id] = static_cast<void*>(&object);
            }
            else {
                // TODO : think about removing this completely. this comments may annoy, while using 'fe::context::emplace_or_replace()'
                //
                //fe::logging::error("Failed to replace an object in the context. There is nothing to replace\nTag %llu\nType index was %llu",
                //                   static_cast<uint64_t>(tag),
                //                   static_cast<uint64_t>(type_id));
                return false;
            }

            return true;
        }

        // @brief emplace if there is no object yet and replace if it is already exists in the context
        // keep in mind that this context won't release your object - you're responsible for its lifetime
        //
        // @tparam T type of your object
        //
        // @param object lvalue of your object
        // @param tag optional tag to distinguish multiple objects of the same type ( 0 by default )
        template <typename T>
        void emplace_or_replace(T& object, fe::string_hash tag = {}) {
            (!this->replace<T>(object, tag)) ? this->emplace<T>(object, tag) : 0;
        }

        // @brief get the object from the context
        //
        // @tparam T type of your object
        //
        // @param tag optional tag to distinguish multiple objects of the same type ( 0 by default )
        //
        // @returns lvalue of your object or calls 'fe::logging::fatal()' if something went wrong
        template <typename T>
        T& get(fe::string_hash tag = {}) {
            const type_registry<context>::type_id type_id = type_registry<context>::get_id<T>();

            auto it = m_objects.find(tag);
            if (it == m_objects.end()) {
                fe::logging::fatal("Failed to get an object from the context. No types were registered with this hash\nTag %llu\nType index was %llu",
                                   static_cast<uint64_t>(tag),
                                   static_cast<uint64_t>(type_id));
            }
            else {
                types& storage = it->second;
                if (type_id >= storage.size() || storage[type_id] == nullptr) {
                    fe::logging::fatal("Failed to find a type. Type wasn't registered in this Tag\nTag %llu\nType index was %llu",
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
