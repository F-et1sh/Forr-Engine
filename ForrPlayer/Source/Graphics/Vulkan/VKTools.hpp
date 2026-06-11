/*===============================================

    Forr Engine

    File : VKTools.hpp
    Role : helper functions like getQueueFamilyIndex() or getMemoryType()

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/logging.hpp"
#include "VulkanContext.hpp"

namespace fe {
    static uint32_t getQueueFamilyIndex(const VulkanContext& context, VkQueueFlags queue_flags) {
        if ((queue_flags & VK_QUEUE_COMPUTE_BIT) == queue_flags) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(context.queue_family_properties.size()); i++) {
                if ((context.queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                    ((context.queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                    return i;
                }
            }
        }

        if ((queue_flags & VK_QUEUE_TRANSFER_BIT) == queue_flags) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(context.queue_family_properties.size()); i++) {
                if ((context.queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                    ((context.queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                    ((context.queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                    return i;
                }
            }
        }

        for (uint32_t i = 0; i < static_cast<uint32_t>(context.queue_family_properties.size()); i++) {
            if ((context.queue_family_properties[i].queueFlags & queue_flags) == queue_flags) {
                return i;
            }
        }

        fe::logging::fatal("Could not find a matching queue family index");

        return 0;
    }

    static uint32_t getMemoryType(const VulkanContext& context, uint32_t type_bits, VkMemoryPropertyFlags properties, VkBool32* memory_type_found = nullptr) {
        const auto& device_properties = context.physical_device_memory_properties;

        for (uint32_t i = 0; i < device_properties.memoryTypeCount; i++) {
            if (type_bits & 1) {
                if ((device_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                    if (memory_type_found) {
                        *memory_type_found = true;
                    }
                    return i;
                }
            }
            type_bits >>= 1;
        }

        if (memory_type_found) {
            *memory_type_found = false;
            return 0;
        }

        fe::logging::fatal("Could not find a matching memory type");

        return 0;
    }

    static uint32_t getMemoryTypeIndex(const VulkanContext& context, uint32_t type_bits, VkMemoryPropertyFlags properties) {
        const auto& device_properties = context.physical_device_memory_properties;

        for (uint32_t i = 0; i < device_properties.memoryTypeCount; i++) {
            if (type_bits & 1) {
                if ((device_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            type_bits >>= 1;
        }

        fe::logging::fatal("Could not find a matching memory type");

        return 0;
    }

    static vk::VmaBuffer createBuffer(VmaAllocator             allocator,
                                      VkDeviceSize             size,
                                      VkBufferUsageFlags       usage,
                                      VmaAllocationCreateFlags vma_allocation_flags) {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size        = size;
        buffer_info.usage       = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_info.flags = vma_allocation_flags;

        VkBuffer      buffer_raw{};
        VmaAllocation allocation_raw{};

        VK_CHECK_RESULT(vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer_raw, &allocation_raw, nullptr));

        return vk::VmaBuffer{ allocator, buffer_raw, allocation_raw };
    }

    template <typename Func>
    static void runOneTimeCommands(VulkanContext& context, Func&& func) {
        vk::CommandBuffer command_buffer{};

        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool        = context.command_pool;
        command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer_raw{};
        VK_CHECK_RESULT(vkAllocateCommandBuffers(context.device, &command_buffer_allocate_info, &command_buffer_raw));
        command_buffer.attach(context.device, context.command_pool, command_buffer);

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer_raw, &command_buffer_begin_info));

        func(command_buffer_raw);

        VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer_raw));

        VkSubmitInfo submit_info{};
        submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &command_buffer_raw;

        vk::Fence fence{};

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence fence_raw{};
        VK_CHECK_RESULT(vkCreateFence(context.device, &fence_create_info, nullptr, &fence_raw));
        fence.attach(context.device, fence_raw);

        VK_CHECK_RESULT(vkQueueSubmit(context.queue_graphics, 1, &submit_info, fence_raw));
        VK_CHECK_RESULT(vkWaitForFences(context.device, 1, &fence_raw, VK_TRUE, context.default_fence_timeout));

        vkDestroyFence(context.device, fence_raw, nullptr);
    }

    static vk::VmaBuffer createDeviceLocalBuffer(VulkanContext&     context,
                                                 const void*        data,
                                                 VkDeviceSize       size,
                                                 VkBufferUsageFlags usage) {
        vk::VmaBuffer staging_buffer = createBuffer(context.allocator,
                                                    size,
                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

        VmaAllocationInfo allocation_info{};
        vmaGetAllocationInfo(context.allocator, staging_buffer.get<VmaAllocation>(), &allocation_info);

        std::memcpy(allocation_info.pMappedData, data, size);

        vk::VmaBuffer device_buffer = createBuffer(context.allocator, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0);

        runOneTimeCommands(context, [&](VkCommandBuffer command_buffer) {
            VkBufferCopy copy_region{};
            copy_region.size = size;
            vkCmdCopyBuffer(command_buffer, staging_buffer.get<VkBuffer>(), device_buffer.get<VkBuffer>(), 1, &copy_region);
        });

        return device_buffer;
    }

} // namespace fe
