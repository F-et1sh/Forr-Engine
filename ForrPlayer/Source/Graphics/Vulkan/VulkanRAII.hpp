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
        void operator()(VkDevice device) const noexcept {
            vkDestroyDevice(device, nullptr);
        }
    };

    struct InstanceDestroy {
        void operator()(VkInstance instance) const noexcept {
            vkDestroyInstance(instance, nullptr);
        }
    };

    struct AllocatorDestroy {
        void operator()(VmaAllocator allocator) const noexcept {
            vmaDestroyAllocator(allocator);
        }
    };

    struct SurfaceDestroy {
        void operator()(VkInstance instance, VkSurfaceKHR handle) const noexcept {
            vkDestroySurfaceKHR(instance, handle, nullptr);
        }
    };

    struct SwapchainDestroy {
        void operator()(VkDevice device, VkSwapchainKHR handle) const noexcept {
            vkDestroySwapchainKHR(device, handle, nullptr);
        }
    };

    struct BufferDestroy {
        void operator()(VkDevice device, VkBuffer handle) const noexcept {
            vkDestroyBuffer(device, handle, nullptr);
        }
    };

    struct ImageDestroy {
        void operator()(VkDevice device, VkImage handle) const noexcept {
            vkDestroyImage(device, handle, nullptr);
        }
    };

    struct ImageViewDestroy {
        void operator()(VkDevice device, VkImageView handle) const noexcept {
            vkDestroyImageView(device, handle, nullptr);
        }
    };

    struct SamplerDestroy {
        void operator()(VkDevice device, VkSampler handle) const noexcept {
            vkDestroySampler(device, handle, nullptr);
        }
    };

    struct ShaderModuleDestroy {
        void operator()(VkDevice device, VkShaderModule handle) const noexcept {
            vkDestroyShaderModule(device, handle, nullptr);
        }
    };

    struct RenderPassDestroy {
        void operator()(VkDevice device, VkRenderPass handle) const noexcept {
            vkDestroyRenderPass(device, handle, nullptr);
        }
    };

    struct FramebufferDestroy {
        void operator()(VkDevice device, VkFramebuffer handle) const noexcept {
            vkDestroyFramebuffer(device, handle, nullptr);
        }
    };

    struct PipelineDestroy {
        void operator()(VkDevice device, VkPipeline handle) const noexcept {
            vkDestroyPipeline(device, handle, nullptr);
        }
    };

    struct PipelineCacheDestroy {
        void operator()(VkDevice device, VkPipelineCache handle) const noexcept {
            vkDestroyPipelineCache(device, handle, nullptr);
        }
    };

    struct PipelineLayoutDestroy {
        void operator()(VkDevice device, VkPipelineLayout handle) const noexcept {
            vkDestroyPipelineLayout(device, handle, nullptr);
        }
    };

    struct DescriptorSetLayoutDestroy {
        void operator()(VkDevice device, VkDescriptorSetLayout handle) const noexcept {
            vkDestroyDescriptorSetLayout(device, handle, nullptr);
        }
    };

    struct DescriptorPoolDestroy {
        void operator()(VkDevice device, VkDescriptorPool handle) const noexcept {
            vkDestroyDescriptorPool(device, handle, nullptr);
        }
    };

    struct CommandPoolDestroy {
        void operator()(VkDevice device, VkCommandPool handle) const noexcept {
            vkDestroyCommandPool(device, handle, nullptr);
        }
    };

    struct FenceDestroy {
        void operator()(VkDevice device, VkFence handle) const noexcept {
            vkDestroyFence(device, handle, nullptr);
        }
    };

    struct SemaphoreDestroy {
        void operator()(VkDevice device, VkSemaphore handle) const noexcept {
            vkDestroySemaphore(device, handle, nullptr);
        }
    };

    struct EventDestroy {
        void operator()(VkDevice device, VkEvent handle) const noexcept {
            vkDestroyEvent(device, handle, nullptr);
        }
    };

    struct DeviceMemoryDestroy {
        void operator()(VkDevice device, VkDeviceMemory handle) const noexcept {
            vkFreeMemory(device, handle, nullptr);
        }
    };

    struct VmaBufferDestroy {
        void operator()(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation) const noexcept {
            vmaDestroyBuffer(allocator, buffer, allocation);
        }
    };

    struct VmaImageDestroy {
        void operator()(VmaAllocator allocator, VkImage image, VmaAllocation allocation) const noexcept {
            vmaDestroyImage(allocator, image, allocation);
        }
    };

    struct CommandBufferDestroy {
        void operator()(VkDevice device, VkCommandPool command_pool, VkCommandBuffer handle) const noexcept {
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
