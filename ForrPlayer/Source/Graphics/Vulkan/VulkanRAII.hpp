/*===============================================

    Forr Engine

    File : VulkanRAII.cpp
    Role : Thin wrapper classes to provide RAII to Vulkan.
        fe::vk:: - is a namespace that contains only thin wrappers.
        Classes/Structs like VulkanDevice won't be here, they will be in fe::

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <span>

#include "Core/logging.hpp"
#include "Core/attributes.hpp"

#include <Volk/volk.h>

#include "vk_mem_alloc.h"

namespace fe::vk {
    template <typename Handle, typename DestroyFn> // unified class for objects, which can destroyed by themselves
    class RootHandle {
    public:
        RootHandle() = default;
        explicit RootHandle(Handle handle) noexcept : handle(handle) {}

        ~RootHandle() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(RootHandle)

        RootHandle(RootHandle&& other) noexcept : handle(other.handle) {
            other.handle = VK_NULL_HANDLE;
        }

        RootHandle& operator=(RootHandle&& other) noexcept {
            if (this != &other) {
                this->attach(other.handle);
                other.handle = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (handle != nullptr) {
                DestroyFn{}(handle);
                handle = VK_NULL_HANDLE;
            }
        }

        void attach(Handle handle) noexcept {
            if (this->handle != handle) {
                this->reset();
                this->handle = handle;
            }
        }

        FORR_NODISCARD Handle get() const noexcept { return handle; }

        operator Handle() const noexcept { return handle; }

    private:
        Handle handle = VK_NULL_HANDLE;
    };

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

    using Device    = RootHandle<VkDevice, DeviceDestroy>;
    using Instance  = RootHandle<VkInstance, InstanceDestroy>;
    using Allocator = RootHandle<VmaAllocator, AllocatorDestroy>;

    ///

    template <typename ParentHandle, typename Handle, typename DestroyFn> // unified class for objects, which needs a parent handle to be destroyed
    class ChildHandle {
    public:
        ChildHandle() = default;
        explicit ChildHandle(ParentHandle parent_handle, Handle handle) noexcept : parent_handle(parent_handle), handle(handle) {}

        ~ChildHandle() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(ChildHandle)

        ChildHandle(ChildHandle&& other) noexcept : parent_handle(other.parent_handle), handle(other.handle) {
            other.parent_handle = VK_NULL_HANDLE;
            other.handle        = VK_NULL_HANDLE;
        }

        ChildHandle& operator=(ChildHandle&& other) noexcept {
            if (this != &other) {
                this->attach(other.parent_handle, other.handle);
                other.parent_handle = VK_NULL_HANDLE;
                other.handle        = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (handle) {
                assert(parent_handle);

                DestroyFn{}(parent_handle, handle);

                parent_handle = VK_NULL_HANDLE;
                handle        = VK_NULL_HANDLE;
            }
        }

        void attach(ParentHandle parent_handle, Handle handle) noexcept {
            assert(this->handle != handle);

            this->reset();

            this->parent_handle = parent_handle;
            this->handle        = handle;
        }

        FORR_NODISCARD Handle       get() const noexcept { return handle; }
        FORR_NODISCARD ParentHandle get_parent_handle() const noexcept { return parent_handle; }

        operator Handle() const noexcept { return handle; }

    protected:
        ParentHandle parent_handle = VK_NULL_HANDLE;
        Handle       handle        = VK_NULL_HANDLE;
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

    using Surface             = ChildHandle<VkInstance, VkSurfaceKHR, SurfaceDestroy>;
    using Swapchain           = ChildHandle<VkDevice, VkSwapchainKHR, SwapchainDestroy>;
    using Buffer              = ChildHandle<VkDevice, VkBuffer, BufferDestroy>;
    using Image               = ChildHandle<VkDevice, VkImage, ImageDestroy>;
    using ImageView           = ChildHandle<VkDevice, VkImageView, ImageViewDestroy>;
    using Sampler             = ChildHandle<VkDevice, VkSampler, SamplerDestroy>;
    using ShaderModule        = ChildHandle<VkDevice, VkShaderModule, ShaderModuleDestroy>;
    using RenderPass          = ChildHandle<VkDevice, VkRenderPass, RenderPassDestroy>;
    using Framebuffer         = ChildHandle<VkDevice, VkFramebuffer, FramebufferDestroy>;
    using Pipeline            = ChildHandle<VkDevice, VkPipeline, PipelineDestroy>;
    using PipelineCache       = ChildHandle<VkDevice, VkPipelineCache, PipelineCacheDestroy>;
    using PipelineLayout      = ChildHandle<VkDevice, VkPipelineLayout, PipelineLayoutDestroy>;
    using DescriptorSetLayout = ChildHandle<VkDevice, VkDescriptorSetLayout, DescriptorSetLayoutDestroy>;
    using DescriptorPool      = ChildHandle<VkDevice, VkDescriptorPool, DescriptorPoolDestroy>;
    using CommandPool         = ChildHandle<VkDevice, VkCommandPool, CommandPoolDestroy>;
    using Fence               = ChildHandle<VkDevice, VkFence, FenceDestroy>;
    using Semaphore           = ChildHandle<VkDevice, VkSemaphore, SemaphoreDestroy>;
    using Event               = ChildHandle<VkDevice, VkEvent, EventDestroy>;
    using DeviceMemory        = ChildHandle<VkDevice, VkDeviceMemory, DeviceMemoryDestroy>;

} // namespace fe::vk
