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
    uint32_t getQueueFamilyIndex(const VulkanContext& context, VkQueueFlags queue_flags) {
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
    }
} // namespace fe
