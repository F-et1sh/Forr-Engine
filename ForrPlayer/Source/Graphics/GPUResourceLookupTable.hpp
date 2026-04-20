/*===============================================

    Forr Engine

    File : GPUResourceLookupTable.hpp
    Role : unified lookup table for CPU <-> GPU communication

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include <unordered_map>

namespace fe {
    // the table invokes CPU and returns GPU pointers
    // all pointers are packed
    class GPUResourceLookupTable {
    public:
        GPUResourceLookupTable()  = default;
        ~GPUResourceLookupTable() = default;

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
                return m_Materials;
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
                return m_Materials;
            }
            else {
                // TODO : Add fallbacks
                assert(false);
                return {};
            }
        }

    private:
        std::unordered_map<fe::pointer<resource::Texture>, uint64_t>  m_Textures{};
        std::unordered_map<fe::pointer<resource::Model>, uint64_t>    m_Models{};
        std::unordered_map<fe::pointer<resource::Material>, uint64_t> m_Materials{};
    };
} // namespace fe
