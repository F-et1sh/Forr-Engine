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

        FORR_NODISCARD fe::pointer<VulkanTexture> CreateResource(const resource::Texture& texture);
        FORR_NODISCARD fe::pointer<VulkanModel> CreateResource(const resource::Model& model);

    private:
        FORR_NODISCARD fe::pointer<VulkanMesh> createMesh(const resource::Model::Mesh& mesh);

    private:
        ResourceManager&       m_ResourceManager;
        VulkanResourceStorage& m_Storage;
    };
} // namespace fe
