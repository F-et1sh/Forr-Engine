/*===============================================

    Forr Engine

    File : VulkanContext.hpp
    Role : VulkanContext just a container for Vulkan stuff aka state.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <vector>
#include <string>
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

        VkDevice       device{};
        VkSurfaceKHR   surface{};
        VkSwapchainKHR swapchain{};

        VkExtent2D swapchain_extent{};
        VkFormat   swapchain_format{};
        uint32_t   swapchain_image_count{};

        std::vector<std::string> supported_instance_extensions{};
        std::vector<std::string> supported_device_extensions{};

        VkPhysicalDeviceFeatures enabled_physical_device_features{};
        std::vector<std::string> enabled_physical_device_extensions{};
        std::vector<std::string> enabled_instance_extensions{};

        std::vector<VkLayerSettingEXT> enabled_layer_settings{};

        void* physical_device_create_next_chain{}; // TODO : this is unused for now. Add adding extra features in the future

        std::vector<VkQueueFamilyProperties> queue_family_properties{};

        VkQueue queue_graphics{};
        VkQueue queue_compute{};
        VkQueue queue_transfer{};

        QueueFamilyIndices queue_family_indices{};

        VkCommandPool command_pool{};

        constexpr inline static uint32_t          api_version           = VK_API_VERSION_1_3;                 // hardcoded for now
        constexpr inline static size_t            MAX_CONCURRENT_FRAMES = 2;                                  // hardcoded for now
        constexpr inline static bool              requires_stencil{ false };                                  // hardcoded for now
        constexpr inline static VkClearColorValue default_clear_color = { { 0.025f, 0.025f, 0.025f, 1.0f } }; // hardcoded for now

        VkFormat depth_format{ VK_FORMAT_UNDEFINED };

        VulkanContext()  = default;
        ~VulkanContext() = default;
    };
} // namespace fe
