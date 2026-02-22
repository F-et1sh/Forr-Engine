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
    struct QueueFamilyIndices {
        uint32_t graphics{};
        uint32_t compute{};
        uint32_t transfer{};

        QueueFamilyIndices()  = default;
        ~QueueFamilyIndices() = default;
    };

    class VulkanDevice {
    public:
        VulkanDevice()  = default;
        ~VulkanDevice() = default;

        void Initialize(VkPhysicalDevice physical_device);

        uint32_t        getMemoryType(uint32_t type_bits, VkMemoryPropertyFlags properties, VkBool32* mem_type_found = nullptr) const;
        uint32_t        getQueueFamilyIndex(VkQueueFlags queue_flags) const;
        VkResult        createLogicalDevice(VkPhysicalDeviceFeatures enabled_features, std::vector<const char*> enabled_extensions, void* p_next_chain, bool use_swap_chain = true, VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
        VkCommandPool   createCommandPool(uint32_t queue_family_index, VkCommandPoolCreateFlags create_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void            flushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool pool, bool free = true);
        void            flushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue, bool free = true);
        bool            extensionSupported(std::string extension);
        VkFormat        getSupportedDepthFormat(bool check_sampling_support);

    private:
        fe::vk::Device m_Device;

        VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };

        VkPhysicalDeviceProperties           m_Properties{};
        VkPhysicalDeviceMemoryProperties     m_MemoryProperties{};
        std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;

        VkPhysicalDeviceFeatures m_Features{};
        VkPhysicalDeviceFeatures m_EnabledFeatures{};

        std::vector<std::string> m_SupportedExtensions;

        fe::vk::CommandPool m_CommandPool;

        QueueFamilyIndices m_QueueFamilyIndices;
    };
} // namespace fe
