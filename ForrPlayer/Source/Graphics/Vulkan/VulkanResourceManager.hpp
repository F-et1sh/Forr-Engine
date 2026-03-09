/*===============================================

    Forr Engine

    File : VulkanResourceManager.hpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/types.hpp"
#include "VKStructures.hpp"
#include "VulkanContext.hpp"

namespace fe {
    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context) : m_Context(context) {}
        ~VulkanResourceManager() = default;

    private:
        VulkanContext& m_Context;

        std::vector<VulkanMesh> m_Meshes;
    };
} // namespace fe
