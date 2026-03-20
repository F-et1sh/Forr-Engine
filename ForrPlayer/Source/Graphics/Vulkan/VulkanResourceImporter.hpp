/*===============================================

    Forr Engine

    File : VulkanResourceImporter.hpp
    Role : creates Vulkan resources from unified ones

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "VulkanResourceStorage.hpp"

namespace fe {
    class VulkanResourceImporter {
    public:
        VulkanResourceImporter(ResourceManager& resource_manager, VulkanResourceStorage& storage)
            : m_ResourceManager(resource_manager), m_Storage(storage) {}
        ~VulkanResourceImporter() = default;

    private:
        ResourceManager&       m_ResourceManager;
        VulkanResourceStorage& m_Storage;
    };
} // namespace fe
