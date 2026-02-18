/*===============================================

    Forr Engine

    File : VulkanDevice.cpp
    Role : VulkanDevice - is a class, where I put physical and logical devices
        and some helper functions 

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanDevice.hpp"

void fe::VulkanDevice::Initialize(VkPhysicalDevice physical_device) {
    assert(physical_device);
    m_PhysicalDevice = physical_device;

    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);

    uint32_t queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, nullptr);
    assert(queue_family_count > 0);
    m_QueueFamilyProperties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, m_QueueFamilyProperties.data());

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extension_count, nullptr);

    if (extension_count > 0) {
        std::vector<VkExtensionProperties> extensions(extension_count);

        VkResult result = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extension_count, &extensions.front());

        if (result == VK_SUCCESS) {
            for (auto& e : extensions) {
                m_SupportedExtensions.push_back(e.extensionName);
            }
        }
    }
}
