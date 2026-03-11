/*===============================================

    Forr Engine

    File : VulkanResourceManager.hpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/types.hpp"
#include "ResourceManagement/ResourceManager.hpp"
#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"

namespace fe {
    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context, ResourceManager& resource_manager)
            : m_Context(context), m_ResourceManager(resource_manager) {}
        ~VulkanResourceManager() = default;

        void CreateTexture(fe::pointer<fe::resource::Texture> cpu_texture_ptr);
        void CreateModel(fe::pointer<fe::resource::Model> cpu_model_ptr);

        template <typename GPU, typename CPU>
        const GPU* GetResource(fe::pointer<CPU> cpu_ptr) {
            const auto& lookup_table = this->GetLookupTable<CPU, GPU>();

            auto it = lookup_table.find(cpu_ptr.packed());
            assert(it != lookup_table.end());

            const auto& storage = this->GetStorage<GPU>();

            fe::pointer<GPU> gpu_ptr{};
            gpu_ptr.unpack(it->second);

            assert(storage.is_valid(gpu_ptr));

            return storage.get(gpu_ptr);
        }

        // unsafe helper function
        template <typename T>
        fe::typed_pointer_storage<T>& GetStorage() {
            if constexpr (std::is_same_v<T, VulkanTexture>) {
                return m_Textures;
            }
            else if constexpr (std::is_same_v<T, VulkanModel>) {
                return m_Models;
            }
            else if constexpr (std::is_same_v<T, VulkanMesh>) {
                return m_Meshes;
            }
            else {
                assert(false);
            }
        }

        template <typename CPU, typename GPU>
        const std::unordered_map<uint64_t, uint64_t>& GetLookupTable() const {
            if constexpr (std::is_same_v<CPU, fe::resource::Texture> &&
                          std::is_same_v<GPU, VulkanTexture>) {
                return m_CPU_GPU_Texture;
            }
            else if constexpr (std::is_same_v<CPU, fe::resource::Model> &&
                               std::is_same_v<GPU, VulkanModel>) {
                return m_CPU_GPU_Model;
            }
            else {
                assert(false);
            }
        }

    private: // helper functions
        fe::pointer<VulkanMesh>    createMesh(const Mesh& mesh);
        fe::pointer<VulkanTexture> createTexture(fe::pointer<fe::resource::Texture> cpu_texture_ptr);
        void                       createPrimitive(const Primitive& primitive, VulkanPrimitive& vulkan_primitive);

    private:
        VulkanContext&   m_Context;
        ResourceManager& m_ResourceManager;

        std::unordered_map<uint64_t, uint64_t> m_CPU_GPU_Texture{};
        std::unordered_map<uint64_t, uint64_t> m_CPU_GPU_Model{};

        fe::typed_pointer_storage<VulkanTexture> m_Textures{};
        fe::typed_pointer_storage<VulkanModel>   m_Models{};
        fe::typed_pointer_storage<VulkanMesh>    m_Meshes{};
    };
} // namespace fe
