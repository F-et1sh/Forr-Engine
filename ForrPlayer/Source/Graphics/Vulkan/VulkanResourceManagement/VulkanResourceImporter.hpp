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

        template <typename T>
        void ImportResource(fe::pointer<T> cpu_side_pointer) {
            const auto& resource = *m_ResourceManager.GetResource(cpu_side_pointer);

            if constexpr (std::is_same_v<T, resource::Texture>) {

            }
            else if constexpr (std::is_same_v<T, resource::Material>) {
            }
            else if constexpr (std::is_same_v<T, resource::Model>) {
            }
            else
                constexpr assert(false);
        }

    private:
        ResourceManager&       m_ResourceManager;
        VulkanResourceStorage& m_Storage;
    };
} // namespace fe
