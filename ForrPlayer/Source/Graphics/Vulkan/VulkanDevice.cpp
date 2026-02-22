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

uint32_t fe::VulkanDevice::getMemoryType(uint32_t type_bits, VkMemoryPropertyFlags properties, VkBool32* mem_type_found) const {
    for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++) {
        if ((type_bits & 1) == 1) {
            if ((m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                if (mem_type_found != nullptr) {
                    *mem_type_found = 1u;
                }
                return i;
            }
        }
        type_bits >>= 1;
    }

    if (mem_type_found != nullptr) {
        *mem_type_found = 0u;
        return 0;
    }

    throw std::runtime_error("Could not find a matching memory type");
}

uint32_t fe::VulkanDevice::getQueueFamilyIndex(VkQueueFlags queue_flags) const {
    if ((queue_flags & VK_QUEUE_COMPUTE_BIT) == queue_flags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++) {
            if (((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u) && ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    if ((queue_flags & VK_QUEUE_TRANSFER_BIT) == queue_flags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++) {
            if (((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u) && ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    for (uint32_t i = 0; i < static_cast<uint32_t>(m_QueueFamilyProperties.size()); i++) {
        if ((m_QueueFamilyProperties[i].queueFlags & queue_flags) == queue_flags) {
            return i;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

VkResult fe::VulkanDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabled_features, std::vector<const char*> enabled_extensions, void* p_next_chain, bool use_swap_chain, VkQueueFlags requested_queue_types) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
    const float                          default_queue_priority(0.0F);

    if ((requested_queue_types & VK_QUEUE_GRAPHICS_BIT) != 0u) {
        m_QueueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queue_info{
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_QueueFamilyIndices.graphics,
            .queueCount       = 1,
            .pQueuePriorities = &default_queue_priority
        };
        queue_create_infos.push_back(queue_info);
    }
    else {
        m_QueueFamilyIndices.graphics = 0;
    }

    if ((requested_queue_types & VK_QUEUE_COMPUTE_BIT) != 0u) {
        m_QueueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (m_QueueFamilyIndices.compute != m_QueueFamilyIndices.graphics) {
            VkDeviceQueueCreateInfo queue_info{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_QueueFamilyIndices.compute,
                .queueCount       = 1,
                .pQueuePriorities = &default_queue_priority,
            };
            queue_create_infos.push_back(queue_info);
        }
    }
    else {
        m_QueueFamilyIndices.compute = m_QueueFamilyIndices.graphics;
    }

    if ((requested_queue_types & VK_QUEUE_TRANSFER_BIT) != 0u) {
        m_QueueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((m_QueueFamilyIndices.transfer != m_QueueFamilyIndices.graphics) && (m_QueueFamilyIndices.transfer != m_QueueFamilyIndices.compute)) {
            VkDeviceQueueCreateInfo queue_info{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_QueueFamilyIndices.transfer,
                .queueCount       = 1,
                .pQueuePriorities = &default_queue_priority
            };
            queue_create_infos.push_back(queue_info);
        }
    }
    else {
        m_QueueFamilyIndices.transfer = m_QueueFamilyIndices.graphics;
    }

    std::vector<const char*> deviceExtensions(enabled_extensions);
    if (use_swap_chain) {
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkDeviceCreateInfo device_create_info{
        .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos    = queue_create_infos.data(),
        .pEnabledFeatures     = &enabled_features
    };

    VkPhysicalDeviceFeatures2 physical_device_features2{};
    if (p_next_chain != nullptr) {
        physical_device_features2.sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physical_device_features2.features  = enabled_features;
        physical_device_features2.pNext     = p_next_chain;
        device_create_info.pEnabledFeatures = nullptr;
        device_create_info.pNext            = &physical_device_features2;
    }

    if (!deviceExtensions.empty()) {
        for (const char* enabled_extension : deviceExtensions) {
            if (!extensionSupported(enabled_extension)) {
                fe::logging::error("Enabled device extension %s is not present at device level", enabled_extension);
            }
        }

        device_create_info.enabledExtensionCount   = (uint32_t) deviceExtensions.size();
        device_create_info.ppEnabledExtensionNames = deviceExtensions.data();
    }

    this->m_EnabledFeatures = enabled_features;

    VkDevice device{};
    VkResult result = vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &device);
    if (result != VK_SUCCESS) {
        return result;
    }
    m_Device.attach(device);

    VkCommandPool command_pool{};
    command_pool = createCommandPool(m_QueueFamilyIndices.graphics);
    m_CommandPool.attach(m_Device, command_pool);

    return result;
}

VkCommandPool fe::VulkanDevice::createCommandPool(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo command_pool_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = create_flags,
        .queueFamilyIndex = queue_family_index
    };
    VkCommandPool command_pool{};
    VK_CHECK_RESULT(vkCreateCommandPool(m_Device, &command_pool_info, nullptr, &command_pool));
    return command_pool;
}

VkCommandBuffer fe::VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin) {
    //VkCommandBufferAllocateInfo command_buffer_allocate_info = vks::initializers::commandBufferAllocateInfo(pool, level, 1);
    VkCommandBuffer command_buffer{};
    //VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, &command_buffer));
    //// If requested, also start recording for the new command buffer
    //if (begin) {
    //    VkCommandBufferBeginInfo command_buffer_info = vks::initializers::commandBufferBeginInfo();
    //    VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &command_buffer_info));
    //}
    return command_buffer;
}

VkCommandBuffer fe::VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin) {
    return createCommandBuffer(level, m_CommandPool, begin);
}

void fe::VulkanDevice::flushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool pool, bool free) {
    //if (command_buffer == VK_NULL_HANDLE) {
    //    return;
    //}

    //VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));

    //VkSubmitInfo submit_info{
    //    .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    //    .commandBufferCount = 1,
    //    .pCommandBuffers    = &command_buffer
    //};
    //// Create fence to ensure that the command buffer has finished executing
    //VkFenceCreateInfo fence_info = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
    //VkFence           fence;
    //VK_CHECK_RESULT(vkCreateFence(m_Device, &fence_info, nullptr, &fence));
    //// Submit to the queue
    //VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, fence));
    //// Wait for the fence to signal that command buffer has finished executing
    //VK_CHECK_RESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
    //vkDestroyFence(m_Device, fence, nullptr);
    //if (free) {
    //    vkFreeCommandBuffers(m_Device, pool, 1, &command_buffer);
    //}
}

void fe::VulkanDevice::flushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue, bool free) {
    flushCommandBuffer(command_buffer, queue, m_CommandPool, free);
}

bool fe::VulkanDevice::extensionSupported(std::string extension) {
    return (std::find(m_SupportedExtensions.begin(), m_SupportedExtensions.end(), extension) != m_SupportedExtensions.end());
}

VkFormat fe::VulkanDevice::getSupportedDepthFormat(bool check_sampling_support) {
    // All depth formats may be optional, so we need to find a suitable depth format to use
    std::vector<VkFormat> depth_formats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    for (auto& format : depth_formats) {
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &format_properties);
        // Format must support depth stencil attachment for optimal tiling
        if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0u) {
            if (check_sampling_support) {
                if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0u) {
                    continue;
                }
            }
            return format;
        }
    }
    throw std::runtime_error("Could not find a matching depth format");
}
