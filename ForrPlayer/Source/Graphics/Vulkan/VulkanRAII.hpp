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
    template <typename DestroyFn, typename... Handles>
    class VulkanHandle {
    public:
        static constexpr std::size_t HandlesCount = sizeof...(Handles);

    public:
        VulkanHandle() = default;
        explicit VulkanHandle(Handles... handles) noexcept
            : m_Handles(handles...) {}

        ~VulkanHandle() { this->free(); }

        FORR_CLASS_NONCOPYABLE(VulkanHandle)

        VulkanHandle(VulkanHandle&& other) noexcept
            : m_Handles(std::move(other.m_Handles)) {
            other.reset_handles();
        }

        VulkanHandle& operator=(VulkanHandle&& other) noexcept {
            if (this != &other) {
                this->free();
                m_Handles = std::move(other.m_Handles);
                other.reset_handles(); // NOT other.free()
            }
            return *this;
        }

        void free() noexcept {
            auto target_handle       = this->get_target_handle();
            using target_handle_type = decltype(target_handle); // there is no need in 'std::decay_t<>'

            if (target_handle != target_handle_type{}) {
                std::apply(DestroyFn{}, m_Handles);
                this->reset_handles();
            }
        }

        void attach(Handles... handles) noexcept {
            std::tuple<Handles...> new_handles{ handles... };

            if (m_Handles != new_handles) {
                this->free();
                m_Handles = std::move(new_handles);
            }
        }

        auto get() const noexcept { return get_target_handle(); }

        operator auto() const noexcept { return get_target_handle(); }

        template <typename T>
        auto get() const noexcept { return std::get<T>(m_Handles); }

    private:
        void reset_handles() noexcept {
            m_Handles = std::tuple<Handles...>{ Handles{}... };
        }

        auto get_target_handle() const noexcept {
            return std::get<HandlesCount - 1>(m_Handles);
        }

    private:
        std::tuple<Handles...> m_Handles{ Handles{}... };
    };

    ///

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

    template <typename T>
    struct TraitsFromDestroyFn;

    template <typename ClassType, typename ReturnType, typename... Args>
    struct TraitsFromDestroyFn<ReturnType (ClassType::*)(Args...) const noexcept> {
        template <typename DestroyFn>
        using HandleType = VulkanHandle<DestroyFn, Args...>;
    };

    // If you get error C2938 - 'fe::vk::MakeVulkanHandle' : Failed to specialize alias template :
    // destroy structure's operator() must have the same signature as in 'TraitsFromDestroyFn<ReturnType (ClassType::*)(Args...) const noexcept'
    // don't forget 'const noexcept'
    template <typename DestroyFn>
    using MakeVulkanHandle = typename TraitsFromDestroyFn<decltype(&DestroyFn::operator())>::template HandleType<DestroyFn>;

    using Device              = MakeVulkanHandle<DeviceDestroy>;
    using Instance            = MakeVulkanHandle<InstanceDestroy>;
    using Allocator           = MakeVulkanHandle<AllocatorDestroy>;
    using Surface             = MakeVulkanHandle<SurfaceDestroy>;
    using Swapchain           = MakeVulkanHandle<SwapchainDestroy>;
    using Buffer              = MakeVulkanHandle<BufferDestroy>;
    using Image               = MakeVulkanHandle<ImageDestroy>;
    using ImageView           = MakeVulkanHandle<ImageViewDestroy>;
    using Sampler             = MakeVulkanHandle<SamplerDestroy>;
    using ShaderModule        = MakeVulkanHandle<ShaderModuleDestroy>;
    using RenderPass          = MakeVulkanHandle<RenderPassDestroy>;
    using Framebuffer         = MakeVulkanHandle<FramebufferDestroy>;
    using Pipeline            = MakeVulkanHandle<PipelineDestroy>;
    using PipelineCache       = MakeVulkanHandle<PipelineCacheDestroy>;
    using PipelineLayout      = MakeVulkanHandle<PipelineLayoutDestroy>;
    using DescriptorSetLayout = MakeVulkanHandle<DescriptorSetLayoutDestroy>;
    using DescriptorPool      = MakeVulkanHandle<DescriptorPoolDestroy>;
    using CommandPool         = MakeVulkanHandle<CommandPoolDestroy>;
    using Fence               = MakeVulkanHandle<FenceDestroy>;
    using Semaphore           = MakeVulkanHandle<SemaphoreDestroy>;
    using Event               = MakeVulkanHandle<EventDestroy>;
    using DeviceMemory        = MakeVulkanHandle<DeviceMemoryDestroy>;
    using VmaBuffer           = MakeVulkanHandle<VmaBufferDestroy>;
    using VmaImage            = MakeVulkanHandle<VmaImageDestroy>;

} // namespace fe::vk
