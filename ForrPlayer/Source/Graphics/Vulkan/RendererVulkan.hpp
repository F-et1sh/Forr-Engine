/*===============================================

    Forr Engine

    File : RendererVulkanGL.hpp
    Role : Vulkan Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <array>
#include "Graphics/IRenderer.hpp"

#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Volk/volk.h"

#include "Graphics/GPUTypes.hpp"
#include "VulkanRAII.hpp"

#include "VulkanContext.hpp"
#include "VulkanSwapchain.hpp"
#include "VKTools.hpp"
#include "VulkanTypes.hpp"

#include "Graphics/Camera.hpp"
#include "VulkanResourceManager.hpp"

namespace fe {
    class RendererVulkan : public IRenderer {
    public:
        RendererVulkan(const RendererDesc& desc,
                       IPlatformSystem&    platform_system,
                       size_t              primary_window_index,
                       ResourceManager&    resource_manager);
        ~RendererVulkan();

        void SetClearColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) override;

        void BeginFrame() override;
        void EndFrame(const RenderPacket& render_packet) override;

        void InitializeGPUResources() override;

    private: // Vulkan initialization queue
        // Create Vulkan base :
        // - volk
        // - vulkan instance
        // - debug messanger
        // - vulkan physical device
        // - setup supported depth/stencil format
        void InitializeBase();

        // Create Vulkan logical device :
        // - queue families
        // - extensions
        // - features
        // - logical device
        // - command pool
        // - queues
        void InitializeDevice();

        // Create Vulkan swapchain :
        // - create surface
        // - create swapchain
        void InitializeSwapchain();

        // Create Vulkan command buffers :
        // - create command buffers
        void InitializeCommandBuffers();

        // Create Vulkan synchronization primitives :
        // - create fences
        // - create present complete semaphores
        // - create render complete semaphores
        void InitializeSynchronizationPrimitives();

        // Create Vulkan depth/stencil :
        // - create image
        // - create device memory
        // - create image view
        void InitializeDepthStencil();

        // Create Vulkan render pass :
        // - do nothing if dynamic renderer enabled ( VulkanContext::use_dynamic_rendering )
        // - create render pass
        void InitializeRenderPass();

        // Create Vulkan framebuffers :
        // - do nothing if dynamic renderer enabled ( VulkanContext::use_dynamic_rendering )
        // - create framebuffers
        void InitializeFramebuffers();

        // Create Vulkan storage buffer :
        // - create storage buffer
        void InitializeStorageBuffers();

        // Create Vulkan descriptor objects
        // - create descriptor set layout
        // - create descriptor pool
        // - create descriptor sets
        void InitializeDescriptors();

    private: // Vulkan step-by-step initialization functions
        void VKCreateInstance();
        void VKChoosePhysicalDevice();
        void VKSetupDepthStencilFormat();
        void VKSetupFeatures();
        void VKSetupQueueFamilyProperties();
        void VKSetupSupportedExtensions();
        void VKCreateDevice(bool use_swapchain = true, VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
        void VKCreateCommandPool(VkCommandPoolCreateFlags create_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        void VKSetupQueues();
        void VKSetupDescriptorSetLayout();
        void VKSetupDescriptorPool();
        void VKSetupDescriptorSets();

    private: // Vulkan helper functions
        // get queue family infos for logical device creation and setup m_Context.queue_family_indices
        std::vector<VkDeviceQueueCreateInfo> getQueueFamilyInfos(bool use_swapchain = true, VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

    private: // static functions
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                                                        VkDebugUtilsMessageTypeFlagsEXT             message_type,
                                                                        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                        void*                                       user_data);

    private: // Others
        void configureCamera();
        void resizeWindow();
        void handleRenderQueue(const RenderPacket& render_packet);

    private:
        struct FrameData {
            // here used VkCommandBuffer, which is not RAII because its memory is going to be freed by command pool, which is already a RAII wrapper
            VkCommandBuffer command_buffer{};

            fe::vk::Fence     wait_fence{};
            fe::vk::Semaphore present_complete_semaphore{};

            VulkanShaderBuffer storage_buffer{};
            
            FrameData()  = default;
            ~FrameData() = default;
        };

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;
        ResourceManager& m_ResourceManager;

        VkPhysicalDevice m_PhysicalDevice{};
        fe::vk::Instance m_Instance{};
        fe::vk::Device   m_Device{};

        VulkanContext m_Context{};

        VulkanResourceManager m_VulkanResourceManager{ m_Context, m_ResourceManager };

        RendererDesc                     m_Description{};
        VulkanSwapchain                  m_Swapchain{ m_Description, m_Context, m_PrimaryWindow };
        fe::vk::RenderPass               m_RenderPass{};
        std::vector<fe::vk::Framebuffer> m_Framebuffers{};
        VulkanImage                      m_DepthStencil{};

        fe::vk::CommandPool    m_CommandPool{};
        fe::vk::DescriptorPool m_DescriptorPool{};

        std::array<FrameData, VulkanContext::max_concurrent_frames> m_FrameData{};
        std::vector<fe::vk::Semaphore>                              m_RenderCompleteSemaphores{};

        Camera                           m_Camera{};
        GPUHandle<resource::Material>    m_CurrentMaterial{};
        GPUHandle<resource::Model::Mesh> m_CurrentMesh{};

        uint32_t m_CurrentImageIndex{};
        uint32_t m_CurrentBuffer{};
        uint32_t m_CurrentFrame{};
        uint32_t m_ImageIndex{};
    };
} // namespace fe
