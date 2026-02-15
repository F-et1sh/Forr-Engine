/*===============================================

    Forr Engine

    File : VKTypes.cpp
    Role : Thin wrapper classes to provide RAII to Vulkan

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <span>

#include "Core/logging.hpp"
#include "Core/attributes.hpp"

#include <vulkan/vulkan_core.h>

#include "Tools.hpp"

namespace fe::vk { // TODO : work with this
    class Device {
        Device(VkPhysicalDevice                         physical_device,
               std::span<const VkDeviceQueueCreateInfo> queues,
               std::span<const char* const>             extensions) {

            VkDeviceCreateInfo create_info{
                .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount    = uint32_t(queues.size()),
                .pQueueCreateInfos       = queues.data(),
                .enabledExtensionCount   = uint32_t(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
            };

            VK_CHECK_RESULT(vkCreateDevice(physical_device, &create_info, nullptr, &device))
        }

        ~Device() {
            if (device) vkDestroyDevice(device, nullptr);
        }

        FORR_CLASS_NONCOPYABLE(Device)

        Device(Device&& other) noexcept : device(other.device) {
            other.device = VK_NULL_HANDLE;
        }

        operator VkDevice() const noexcept { return device; }

    private:
        VkDevice device = VK_NULL_HANDLE;
    };

} // namespace fe::vk
