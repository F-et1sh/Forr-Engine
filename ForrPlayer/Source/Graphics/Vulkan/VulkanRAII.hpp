/*===============================================

    Forr Engine

    File : VulkanRAII.cpp
    Role : Thin wrapper classes to provide RAII to Vulkan.
        fe::vk:: - is a namespace that contains only thin wrappers.
        Classes/Structs like VulkanDevice won't be here, they will be in fe::

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once

#include <span>

#include "Forr/Core/unique_handle.hpp"
#include "Forr/Core/logging.hpp"
#include "Forr/Core/attributes.hpp"

#include <Volk/volk.h>

#include "vk_mem_alloc.h"

namespace fe::vk {
    struct DeviceDestroy {
        static void operator()(VkDevice device) noexcept {
            vkDestroyDevice(device, nullptr);
        }
    };

    struct InstanceDestroy {
        static void operator()(VkInstance instance) noexcept {
            vkDestroyInstance(instance, nullptr);
        }
    };

    struct AllocatorDestroy {
        static void operator()(VmaAllocator allocator) noexcept {
            vmaDestroyAllocator(allocator);
        }
    };

    struct SurfaceDestroy {
        static void operator()(VkInstance instance, VkSurfaceKHR handle) noexcept {
            vkDestroySurfaceKHR(instance, handle, nullptr);
        }
    };

    struct SwapchainDestroy {
        static void operator()(VkDevice device, VkSwapchainKHR handle) noexcept {
            vkDestroySwapchainKHR(device, handle, nullptr);
        }
    };

    struct BufferDestroy {
        static void operator()(VkDevice device, VkBuffer handle) noexcept {
            vkDestroyBuffer(device, handle, nullptr);
        }
    };

    struct ImageDestroy {
        static void operator()(VkDevice device, VkImage handle) noexcept {
            vkDestroyImage(device, handle, nullptr);
        }
    };

    struct ImageViewDestroy {
        static void operator()(VkDevice device, VkImageView handle) noexcept {
            vkDestroyImageView(device, handle, nullptr);
        }
    };

    struct SamplerDestroy {
        static void operator()(VkDevice device, VkSampler handle) noexcept {
            vkDestroySampler(device, handle, nullptr);
        }
    };

    struct ShaderModuleDestroy {
        static void operator()(VkDevice device, VkShaderModule handle) noexcept {
            vkDestroyShaderModule(device, handle, nullptr);
        }
    };

    struct RenderPassDestroy {
        static void operator()(VkDevice device, VkRenderPass handle) noexcept {
            vkDestroyRenderPass(device, handle, nullptr);
        }
    };

    struct FramebufferDestroy {
        static void operator()(VkDevice device, VkFramebuffer handle) noexcept {
            vkDestroyFramebuffer(device, handle, nullptr);
        }
    };

    struct PipelineDestroy {
        static void operator()(VkDevice device, VkPipeline handle) noexcept {
            vkDestroyPipeline(device, handle, nullptr);
        }
    };

    struct PipelineCacheDestroy {
        static void operator()(VkDevice device, VkPipelineCache handle) noexcept {
            vkDestroyPipelineCache(device, handle, nullptr);
        }
    };

    struct PipelineLayoutDestroy {
        static void operator()(VkDevice device, VkPipelineLayout handle) noexcept {
            vkDestroyPipelineLayout(device, handle, nullptr);
        }
    };

    struct DescriptorSetLayoutDestroy {
        static void operator()(VkDevice device, VkDescriptorSetLayout handle) noexcept {
            vkDestroyDescriptorSetLayout(device, handle, nullptr);
        }
    };

    struct DescriptorPoolDestroy {
        static void operator()(VkDevice device, VkDescriptorPool handle) noexcept {
            vkDestroyDescriptorPool(device, handle, nullptr);
        }
    };

    struct CommandPoolDestroy {
        static void operator()(VkDevice device, VkCommandPool handle) noexcept {
            vkDestroyCommandPool(device, handle, nullptr);
        }
    };

    struct FenceDestroy {
        static void operator()(VkDevice device, VkFence handle) noexcept {
            vkDestroyFence(device, handle, nullptr);
        }
    };

    struct SemaphoreDestroy {
        static void operator()(VkDevice device, VkSemaphore handle) noexcept {
            vkDestroySemaphore(device, handle, nullptr);
        }
    };

    struct EventDestroy {
        static void operator()(VkDevice device, VkEvent handle) noexcept {
            vkDestroyEvent(device, handle, nullptr);
        }
    };

    struct DeviceMemoryDestroy {
        static void operator()(VkDevice device, VkDeviceMemory handle) noexcept {
            vkFreeMemory(device, handle, nullptr);
        }
    };

    struct VmaBufferDestroy {
        static void operator()(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation) noexcept {
            vmaDestroyBuffer(allocator, buffer, allocation);
        }
    };

    struct VmaImageDestroy {
        static void operator()(VmaAllocator allocator, VkImage image, VmaAllocation allocation) noexcept {
            vmaDestroyImage(allocator, image, allocation);
        }
    };

    struct CommandBufferDestroy {
        static void operator()(VkDevice device, VkCommandPool command_pool, VkCommandBuffer handle) noexcept {
            vkFreeCommandBuffers(device, command_pool, 1, &handle);
        }
    };

    using Device              = fe::create_unique_handle<DeviceDestroy>;
    using Instance            = fe::create_unique_handle<InstanceDestroy>;
    using Allocator           = fe::create_unique_handle<AllocatorDestroy>;
    using Surface             = fe::create_unique_handle<SurfaceDestroy>;
    using Swapchain           = fe::create_unique_handle<SwapchainDestroy>;
    using Buffer              = fe::create_unique_handle<BufferDestroy>;
    using Image               = fe::create_unique_handle<ImageDestroy>;
    using ImageView           = fe::create_unique_handle<ImageViewDestroy>;
    using Sampler             = fe::create_unique_handle<SamplerDestroy>;
    using ShaderModule        = fe::create_unique_handle<ShaderModuleDestroy>;
    using RenderPass          = fe::create_unique_handle<RenderPassDestroy>;
    using Framebuffer         = fe::create_unique_handle<FramebufferDestroy>;
    using Pipeline            = fe::create_unique_handle<PipelineDestroy>;
    using PipelineCache       = fe::create_unique_handle<PipelineCacheDestroy>;
    using PipelineLayout      = fe::create_unique_handle<PipelineLayoutDestroy>;
    using DescriptorSetLayout = fe::create_unique_handle<DescriptorSetLayoutDestroy>;
    using DescriptorPool      = fe::create_unique_handle<DescriptorPoolDestroy>;
    using CommandPool         = fe::create_unique_handle<CommandPoolDestroy>;
    using Fence               = fe::create_unique_handle<FenceDestroy>;
    using Semaphore           = fe::create_unique_handle<SemaphoreDestroy>;
    using Event               = fe::create_unique_handle<EventDestroy>;
    using DeviceMemory        = fe::create_unique_handle<DeviceMemoryDestroy>;
    using VmaBuffer           = fe::create_unique_handle<VmaBufferDestroy>;
    using VmaImage            = fe::create_unique_handle<VmaImageDestroy>;
    using CommandBuffer       = fe::create_unique_handle<CommandBufferDestroy>;

} // namespace fe::vk
