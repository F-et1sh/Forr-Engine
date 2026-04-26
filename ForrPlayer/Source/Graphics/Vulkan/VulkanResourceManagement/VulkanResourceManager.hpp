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
#include "Graphics/Vulkan/VulkanContext.hpp"

#include "VulkanResourceCreator.hpp"

namespace fe {
    template <typename T>
    concept vulkan_resource_t =
        (std::is_same_v<T, VulkanTexture>) ||
        (std::is_same_v<T, VulkanMesh>) ||
        //(std::is_same_v<T, VulkanMaterial>) || // TODO : provide materials
        (std::is_same_v<T, VulkanModel>);

    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context, ResourceManager& resource_manager)
            : m_Context(context), m_ResourceManager(resource_manager) {}
        ~VulkanResourceManager() = default;

        template <resource::resource_t T>
        auto CreateResource(fe::pointer<T> cpu_resource_ptr) {
            const auto& resource = *m_ResourceManager.GetResource(cpu_resource_ptr);
            auto        ptr      = m_Importer.CreateResource(resource); // does not need to store this fe::pointer
            m_LookupTable.Insert(cpu_resource_ptr, ptr);
            return ptr;
        }

        template <vulkan_resource_t T>
        FORR_NODISCARD const T* GetResource(fe::pointer<T> gpu_resource_ptr) const {
            const auto& storage  = m_Storage.GetStorage<T>();
            auto        resource = storage.get(gpu_resource_ptr);
            return resource;
        }

        template <resource::resource_t T>
        FORR_NODISCARD auto GetGPUPointer(fe::pointer<T> cpu_resource_ptr) {
            uint64_t packed = m_LookupTable.GetPackedPointerGPU(cpu_resource_ptr);

            if constexpr (std::is_same_v<T, resource::Texture>) {
                return fe::pointer<VulkanTexture>{ packed };
            }
            else if constexpr (std::is_same_v<T, resource::Model>) {
                return fe::pointer<VulkanModel>{ packed };
            }
            else {
                assert(false);
                return {};
            }
        }

    private:
        VulkanContext&   m_Context;
        ResourceManager& m_ResourceManager;

        VulkanResourceStorage  m_Storage{};
        GPUResourceLookupTable m_LookupTable{};
        VulkanResourceCreator  m_Importer{ m_Context, m_ResourceManager, m_Storage };
    };
} // namespace fe
