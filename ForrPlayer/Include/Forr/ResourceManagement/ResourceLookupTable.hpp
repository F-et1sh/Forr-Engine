/*===============================================

    Forr Engine

    File : ResourceLookupTable.hpp
    Role : lookup table for runtime <-> disc ( explorer ) communication

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include "Core/guid.hpp"
#include <unordered_map>

#include "ResourceManagement/Resources.hpp"

namespace fe {
    class ResourceLookupTable {
    public:
        ResourceLookupTable()  = default;
        ~ResourceLookupTable() = default;

        template <resource::resource_t CPU, typename GPU>
        void Insert(fe::pointer<CPU> cpu_side_pointer, fe::pointer<GPU> gpu_side_pointer) {
            auto& storage = this->GetStorage<CPU>();
            storage.insert({ cpu_side_pointer, gpu_side_pointer.packed() });
        }

        template <resource::resource_t T>
        FORR_NODISCARD uint64_t GetPackedPointerGPU(fe::pointer<T> cpu_side_pointer) const {
            const auto& storage = this->GetStorage<T>();

            auto it = storage.find(cpu_side_pointer);
            if (it != storage.end()) {
                return it->second;
            }

            // TODO : Add fallbacks
            assert(false);
            return {};
        }

    private:
        // const
        template <resource::resource_t T>
        const std::unordered_map<fe::pointer<T>, uint64_t>& GetStorage() const {
            if constexpr (std::is_same_v<T, resource::Texture>) {
                return m_Textures;
            }
            else if constexpr (std::is_same_v<T, resource::Model>) {
                return m_Models;
            }
            else if constexpr (std::is_same_v<T, resource::Material>) {
                //return m_Materials; // TODO : provide materials
            }
            else {
                // TODO : Add fallbacks
                assert(false);
                return {};
            }
        }

        // non-const
        template <resource::resource_t T>
        std::unordered_map<fe::pointer<T>, uint64_t>& GetStorage() {
            if constexpr (std::is_same_v<T, resource::Texture>) {
                return m_Textures;
            }
            else if constexpr (std::is_same_v<T, resource::Model>) {
                return m_Models;
            }
            else if constexpr (std::is_same_v<T, resource::Material>) {
                //return m_Materials; // TODO : provide materials
            }
            else {
                // TODO : Add fallbacks
                assert(false);
                return {};
            }
        }

    private:
        std::unordered_map<fe::GUID, fe::pointer<resource::Texture>>  m_Textures{};
        std::unordered_map<fe::GUID, fe::pointer<resource::Model>>    m_Models{};
        std::unordered_map<fe::GUID, fe::pointer<resource::Material>> m_Materials{};
    };
} // namespace fe
