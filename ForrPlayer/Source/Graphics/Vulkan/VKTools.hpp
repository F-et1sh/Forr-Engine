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
                if ((context.queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((context.queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                    return i;
                }
            }
        }

        if ((queue_flags & VK_QUEUE_TRANSFER_BIT) == queue_flags) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(context.queue_family_properties.size()); i++) {
                if ((context.queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((context.queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((context.queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
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
        for (uint32_t i = 0; i < context.physical_device_memory_properties.memoryTypeCount; i++) {
            if (type_bits & 1) {
                if ((context.physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
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
} // namespace fe
