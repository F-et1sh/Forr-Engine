/*===============================================

    Forr Engine

    File : VulkanResourceStorage.hpp
    Role : stores Vulkan resources

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "Graphics/Vulkan/VulkanTypes.hpp"

namespace fe {
    class VulkanResourceStorage {
    public:
        VulkanResourceStorage()  = default;
        ~VulkanResourceStorage() = default;

    private:
        fe::typed_pointer_storage<VulkanTexture> m_Textures{};
        fe::typed_pointer_storage<VulkanModel>   m_Models{};
        fe::typed_pointer_storage<VulkanMesh>    m_Meshes{};
    };
} // namespace fe
