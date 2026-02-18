/*===============================================

    Forr Engine

    File : VulkanDevice.hpp
    Role : VulkanDevice - is a class, where I put physical and logical devices
        and some helper functions 

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string>
#include "VKTypes.hpp"

// namespace is fe::, not fe::vk::
// fe::vk:: - is a namespace that contains only thin wrappers
namespace fe {
    class VulkanDevice {
    public:
        VulkanDevice()  = default;
        ~VulkanDevice() = default;

        void Initialize(VkPhysicalDevice physical_device);

    private:
        fe::vk::Device m_Device;

        VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };

        VkPhysicalDeviceProperties           m_Properties{};
        VkPhysicalDeviceMemoryProperties     m_MemoryProperties{};
        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties{};

        VkPhysicalDeviceFeatures m_Features{};
        VkPhysicalDeviceFeatures m_EnabledFeatures{};

        std::vector<std::string> m_SupportedExtensions{};

        fe::vk::CommandPool m_CommandPool;

        fe::vk::QueueFamilyIndices m_QueueFamilyIndices;
    };
} // namespace fe
