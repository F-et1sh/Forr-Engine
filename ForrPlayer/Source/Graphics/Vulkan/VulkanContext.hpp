/*===============================================

    Forr Engine

    File : VulkanContext.hpp
    Role : VulkanContext just a container for Vulkan stuff aka state.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <vector>
#include "Volk/volk.h"

namespace fe {
    // TODO : Think about adding present
    struct QueueFamilyIndices {
        uint32_t graphics{};
        uint32_t compute{};
        uint32_t transfer{};

        QueueFamilyIndices()  = default;
        ~QueueFamilyIndices() = default;
    };

    struct VulkanContext {
        VkInstance       instance{};
        VkPhysicalDevice physical_device{};

        VkPhysicalDeviceProperties       physical_device_properties{};
        VkPhysicalDeviceFeatures         physical_device_features{};
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties{};

        VkDevice device{};

        std::vector<const char*> supported_instance_extensions{};
        std::vector<const char*> supported_device_extensions{};

        VkPhysicalDeviceFeatures enabled_physical_device_features{};
        std::vector<const char*> enabled_physical_device_extensions{};
        std::vector<const char*> enabled_instance_extensions{};

        std::vector<VkLayerSettingEXT> enabled_layer_settings{};

        void* physical_device_create_next_chain{}; // TODO : this is unused for now. Add adding extra features in the future

        VkSurfaceKHR surface{};

        std::vector<VkQueueFamilyProperties> queue_family_properties{};

        VkQueue queue_graphics{};
        VkQueue queue_compute{};
        VkQueue queue_transfer{};

        QueueFamilyIndices queue_family_indices{};

        VkCommandPool command_pool{};

        constexpr static uint32_t api_version = VK_API_VERSION_1_3; // hardcoded for now

        VulkanContext()  = default;
        ~VulkanContext() = default;
    };
} // namespace fe
