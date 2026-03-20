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
    struct VulkanResourceStorage {
    public:
        fe::typed_pointer_storage<VulkanTexture> m_Textures{};
        fe::typed_pointer_storage<VulkanModel>   m_Models{};
        fe::typed_pointer_storage<VulkanMesh>    m_Meshes{};

        template <typename T>
        const fe::typed_pointer_storage<T>& GetStorage() const {
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
                return {};
            }
        }

        VulkanResourceStorage()  = default;
        ~VulkanResourceStorage() = default;
    };
} // namespace fe
