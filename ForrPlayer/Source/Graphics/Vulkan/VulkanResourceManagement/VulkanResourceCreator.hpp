/*===============================================

    Forr Engine

    File : VulkanResourceCreator.hpp
    Role : creates Vulkan resources from unified ones

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "VulkanResourceStorage.hpp"

namespace fe {
    class VulkanResourceCreator {
    public:
        VulkanResourceCreator(ResourceManager& resource_manager, VulkanResourceStorage& storage)
            : m_ResourceManager(resource_manager), m_Storage(storage) {}
        ~VulkanResourceCreator() = default;

        FORR_NODISCARD fe::pointer<VulkanTexture> CreateTexture(const resource::Texture& texture);

    private:
        ResourceManager&       m_ResourceManager;
        VulkanResourceStorage& m_Storage;
    };
} // namespace fe
