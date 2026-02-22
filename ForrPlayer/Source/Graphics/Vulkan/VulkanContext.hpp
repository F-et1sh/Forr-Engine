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
	struct VulkanContext {
        VkInstance       instance{};
        VkPhysicalDevice physical_device{};

        VkPhysicalDeviceProperties       device_properties{};
        VkPhysicalDeviceFeatures         device_features{};
        VkPhysicalDeviceMemoryProperties device_memory_properties{};

        VkDevice device{};

        VkPhysicalDeviceFeatures enabled_device_features{};
        std::vector<const char*> enabled_device_extensions{};
        std::vector<const char*> enabled_instance_extensions{};

        std::vector<VkLayerSettingEXT> enabled_layer_settings{};

        void* device_create_next_chain{}; // TODO : this is unused for now. Add adding extra features in the future

        VkSurfaceKHR surface{};

        // TODO : think about adding more queues like transfer and compute

        VkQueue graphics_queue{};
        VkQueue present_queue{};

        uint32_t graphics_queue_family{};
        uint32_t present_queue_family{};

        VulkanContext()  = default;
        ~VulkanContext() = default;
    };
}