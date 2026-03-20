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

#include "VulkanResourceImporter.hpp"

namespace fe {
    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context, ResourceManager& resource_manager)
            : m_Context(context), m_ResourceManager(resource_manager) {}
        ~VulkanResourceManager() = default;

        void CreateTexture(fe::pointer<resource::Texture> texture_ptr);

    private:
        VulkanResourceStorage  m_Storage{};
        VulkanResourceImporter m_Importer{ m_ResourceManager, m_Storage };

        VulkanContext&   m_Context;
        ResourceManager& m_ResourceManager;
    };
} // namespace fe
