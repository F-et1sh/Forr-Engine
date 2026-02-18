/*===============================================

    Forr Engine

    File : VKTypes.cpp
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

#include "Tools.hpp"

namespace fe::vk {
    struct Device {
        Device() = default;
        explicit Device(VkDevice device) noexcept : device(device) {}

        ~Device() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(Device)

        Device(Device&& other) noexcept : device(other.device) {
            other.device = VK_NULL_HANDLE;
        }

        Device& operator=(Device&& other) noexcept {
            if (this != &other) {
                this->attach(other.device);
                other.device = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (device) {
                vkDestroyDevice(device, nullptr);
                device = VK_NULL_HANDLE;
            }
        }

        void attach(VkDevice device) noexcept {
            if (this->device != device) {
                this->reset();
                this->device = device;
            }
        }

        FORR_NODISCARD VkDevice get() const noexcept { return device; }

        operator VkDevice() const noexcept { return device; }

    private:
        VkDevice device = VK_NULL_HANDLE;
    };

    struct Instance {
        Instance() = default;
        explicit Instance(VkInstance instance) noexcept : instance(instance) {}

        ~Instance() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(Instance)

        Instance(Instance&& other) noexcept : instance(other.instance) {
            other.instance = VK_NULL_HANDLE;
        }

        Instance& operator=(Instance&& other) noexcept {
            if (this != &other) {
                this->attach(other.instance);
                other.instance = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (instance) {
                vkDestroyInstance(instance, nullptr);
                instance = VK_NULL_HANDLE;
            }
        }

        void attach(VkInstance instance) noexcept {
            if (this->instance != instance) {
                this->reset();
                this->instance = instance;
            }
        }

        FORR_NODISCARD VkInstance get() const noexcept { return instance; }

        operator VkInstance() const noexcept { return instance; }

    private:
        VkInstance instance = VK_NULL_HANDLE;
    };

    template <typename Handle, typename DestroyFn> // unified class for objects, which needs VkDevice
    class DeviceHandle {
    public:
        DeviceHandle() = default;
        explicit DeviceHandle(VkDevice device, Handle handle) noexcept : device(device), handle(handle) {}

        ~DeviceHandle() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(DeviceHandle)

        DeviceHandle(DeviceHandle&& other) noexcept : device(other.device), handle(other.handle) {
            other.device = VK_NULL_HANDLE;
            other.handle = VK_NULL_HANDLE;
        }

        DeviceHandle& operator=(DeviceHandle&& other) noexcept {
            if (this != &other) {
                this->attach(other.device, other.handle);
                other.device = VK_NULL_HANDLE;
                other.handle = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (handle) {
                assert(device);

                DestroyFn{}(device, handle);

                device = VK_NULL_HANDLE;
                handle = VK_NULL_HANDLE;
            }
        }

        void attach(VkDevice device, Handle handle) noexcept {
            if (this->handle != handle ||
                this->device != device) {

                this->reset();

                this->device = device;
                this->handle = handle;
            }
        }

        FORR_NODISCARD Handle   get() const noexcept { return handle; }
        FORR_NODISCARD VkDevice get_device() const noexcept { return device; }

        operator Handle() const noexcept { return handle; }

    protected:
        VkDevice device = VK_NULL_HANDLE;
        Handle   handle = VK_NULL_HANDLE;
    };

    template <typename Handle, typename DestroyFn> // unified class for objects, which needs VkInstance
    class InstanceHandle {
    public:
        InstanceHandle() = default;
        explicit InstanceHandle(VkInstance instance, Handle handle) noexcept : instance(instance), handle(handle) {}

        ~InstanceHandle() { this->reset(); }

        FORR_CLASS_NONCOPYABLE(InstanceHandle)

        InstanceHandle(InstanceHandle&& other) noexcept : instance(other.instance), handle(other.handle) {
            other.instance = VK_NULL_HANDLE;
            other.handle   = VK_NULL_HANDLE;
        }

        InstanceHandle& operator=(InstanceHandle&& other) noexcept {
            if (this != &other) {
                this->attach(other.instance, other.handle);
                other.instance = VK_NULL_HANDLE;
                other.handle   = VK_NULL_HANDLE; // NOT other.reset()
            }
            return *this;
        }

        void reset() noexcept {
            if (handle) {
                assert(instance);

                DestroyFn{}(instance, handle);

                instance = VK_NULL_HANDLE;
                handle   = VK_NULL_HANDLE;
            }
        }

        void attach(VkInstance instance, Handle handle) noexcept {
            if (this->handle != handle ||
                this->instance != instance) {

                this->reset();

                this->instance = instance;
                this->handle   = handle;
            }
        }

        FORR_NODISCARD Handle     get() const noexcept { return handle; }
        FORR_NODISCARD VkInstance get_instance() const noexcept { return instance; }

        operator Handle() const noexcept { return handle; }

    protected:
        VkInstance instance = VK_NULL_HANDLE;
        Handle     handle   = VK_NULL_HANDLE;
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

    using Surface             = InstanceHandle<VkSurfaceKHR, SurfaceDestroy>;
    using Swapchain           = DeviceHandle<VkSwapchainKHR, SwapchainDestroy>;
    using Buffer              = DeviceHandle<VkBuffer, BufferDestroy>;
    using Image               = DeviceHandle<VkImage, ImageDestroy>;
    using ImageView           = DeviceHandle<VkImageView, ImageViewDestroy>;
    using Sampler             = DeviceHandle<VkSampler, SamplerDestroy>;
    using ShaderModule        = DeviceHandle<VkShaderModule, ShaderModuleDestroy>;
    using RenderPass          = DeviceHandle<VkRenderPass, RenderPassDestroy>;
    using Framebuffer         = DeviceHandle<VkFramebuffer, FramebufferDestroy>;
    using Pipeline            = DeviceHandle<VkPipeline, PipelineDestroy>;
    using PipelineLayout      = DeviceHandle<VkPipelineLayout, PipelineLayoutDestroy>;
    using DescriptorSetLayout = DeviceHandle<VkDescriptorSetLayout, DescriptorSetLayoutDestroy>;
    using DescriptorPool      = DeviceHandle<VkDescriptorPool, DescriptorPoolDestroy>;
    using CommandPool         = DeviceHandle<VkCommandPool, CommandPoolDestroy>;
    using Fence               = DeviceHandle<VkFence, FenceDestroy>;
    using Semaphore           = DeviceHandle<VkSemaphore, SemaphoreDestroy>;
    using Event               = DeviceHandle<VkEvent, EventDestroy>;

    // can be removed
    template <typename T, typename Func, typename... Args>
    void create_and_wrap(T& object, VkDevice device, Func&& func, Args&&... args) {
        using VK_T = std::decay_t<decltype(object.get())>;
        VK_T vk_object{};
        VK_CHECK_RESULT(func(device, std::forward<Args>(args)..., &vk_object))
        object.attach(device, vk_object);
    }

} // namespace fe::vk
