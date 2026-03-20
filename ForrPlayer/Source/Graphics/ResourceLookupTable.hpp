/*===============================================

    Forr Engine

    File : ResourceLookupTable.hpp
    Role : unified lookup table for CPU <-> GPU communication

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    // the table invokes CPU and returns GPU pointers
    // all pointers are packed
    class ResourceLookupTable {
    public:
        ResourceLookupTable()  = default;
        ~ResourceLookupTable() = default;

        template <typename CPU, typename GPU>
        void Insert(fe::pointer<CPU> cpu_side_pointer, fe::pointer<GPU> gpu_side_pointer) const {
            const auto& storage = this->GetStorage<CPU>();
            storage.insert({ cpu_side_pointer, gpu_side_pointer.packed() });
        }

        template <typename T>
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
        template <typename T>
        const std::unordered_map<fe::pointer<T>, uint64_t>& GetStorage() const {
            if constexpr (std::is_same_v<T, resource::Texture>) {
                return m_Textures;
            }
            else if constexpr (std::is_same_v<T, resource::Material>) {
                //return m_Materials;
            }
            else
                constexpr {
                    // TODO : Add fallbacks
                    assert(false);
                    return {};
                }
        }

    private:
        std::unordered_map<fe::pointer<resource::Texture>, uint64_t> m_Textures{};
    };
} // namespace fe
