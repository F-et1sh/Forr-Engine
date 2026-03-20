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

#include "Graphics/ResourceLookupTable.hpp"

namespace fe {
    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context, ResourceManager& resource_manager)
            : m_Context(context), m_ResourceManager(resource_manager) {}
        ~VulkanResourceManager() = default;

        void CreateTexture(fe::pointer<resource::Texture> texture_ptr);

        const VulkanTexture* GetTexture(fe::pointer<resource::Texture> texture_ptr) const {
            uint64_t                   packed = m_LookupTable.GetPackedPointerGPU(texture_ptr);
            fe::pointer<VulkanTexture> gpu_pointer{ packed };

            auto texture = m_Storage.m_Textures.get(gpu_pointer);
            return texture;
        }

    private:
        VulkanResourceStorage  m_Storage{};
        ResourceLookupTable    m_LookupTable{};
        VulkanResourceCreator m_Importer{ m_ResourceManager, m_Storage };

        VulkanContext&   m_Context;
        ResourceManager& m_ResourceManager;
    };
} // namespace fe
