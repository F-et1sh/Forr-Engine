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
        RendererVulkan(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index, ResourceManager& resource_manager);
        ~RendererVulkan();

        void SetClearColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) override;

        void BeginFrame() override;
        void Draw(DrawMeshCommand command) override;
        void EndFrame() override;

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

        // Create Vulkan pipeline cache :
        // - create pipeline cache
        void InitializePipelineCache();

        // Create Vulkan framebuffers :
        // - do nothing if dynamic renderer enabled ( VulkanContext::use_dynamic_rendering )
        // - create framebuffers
        void InitializeFramebuffers();

        // Create Vulkan storage buffer :
        // - create storage buffer
        void InitializeStorageBuffer();

        // Create Vulkan descriptor objects
        // - create descriptor set layout
        // - create descriptor pool
        // - create descriptor sets
        void InitializeDescriptors();

        // Create Vulkan pipeline
        // - setup pipeline layout
        // - create pipeline
        void InitializePipeline();

    private: // Vulkan step-by-step initialization functions
        void VKCreateInstance();
        void VKChoosePhysicalDevice();
        void VKSetupDepthStencilFormat();
        void VKSetupQueueFamilyProperties();
        void VKSetupSupportedExtensions();
        void VKCreateDevice(bool use_swapchain = true, VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
        void VKCreateCommandPool(VkCommandPoolCreateFlags create_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        void VKSetupQueues();
        void VKSetupDescriptorSetLayout();
        void VKSetupDescriptorPool();
        void VKSetupDescriptorSets();
        void VKSetupPipelineLayout();

    private: // Vulkan helper functions
        // get queue family infos for logical device creation and setup m_Context.queue_family_indices
        std::vector<VkDeviceQueueCreateInfo> getQueueFamilyInfos(bool use_swapchain = true, VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
        fe::vk::ShaderModule                 createShaderModule(const std::filesystem::path& path);

    private: // static functions
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                                                        VkDebugUtilsMessageTypeFlagsEXT             message_type,
                                                                        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                        void*                                       user_data);

    private:
        void DrawPrimitive(const VulkanVertexBuffer& vertex_buffer, const VulkanIndexBuffer& index_buffer, uint32_t index_offset, uint32_t index_count);

    private: // Others
        void configureCamera();
        void resizeWindow();
        void increaseMeshIndex() noexcept { m_MeshIndex++; }
        void resetMeshIndex() noexcept { m_MeshIndex = 0; }

    private:
        RendererDesc m_Description{};

        ResourceManager& m_ResourceManager;

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        VulkanContext m_Context{};

        fe::vk::Instance m_Instance{};
        VkPhysicalDevice m_PhysicalDevice{}; // not RAII. doesn't need to be destroyed

        fe::vk::Device m_Device{};

        // VkCommandBuffer is not RAII because its memory is going to be freed by command pool,
        // which has RAII wrapper
        std::array<VkCommandBuffer, VulkanContext::max_concurrent_frames> m_CommandBuffers{};

        VulkanSwapchain m_Swapchain{ m_Description, m_Context, m_PrimaryWindow };

        uint32_t m_CurrentImageIndex{};
        uint32_t m_CurrentBuffer{};

        std::array<fe::vk::Fence, VulkanContext::max_concurrent_frames>     m_WaitFences{};
        std::array<fe::vk::Semaphore, VulkanContext::max_concurrent_frames> m_PresentCompleteSemaphores{};
        std::vector<fe::vk::Semaphore>                                      m_RenderCompleteSemaphores{};

        VulkanImage m_DepthStencil{};

        fe::vk::RenderPass m_RenderPass{};

        fe::vk::PipelineCache m_PipelineCache{};

        std::vector<fe::vk::Framebuffer> m_Framebuffers{};

        std::array<VulkanStorageBuffer, VulkanContext::max_concurrent_frames> m_StorageBuffers{};

        fe::vk::DescriptorPool      m_DescriptorPool{};
        fe::vk::DescriptorSetLayout m_DescriptorSetLayout{};

        fe::vk::PipelineLayout m_PipelineLayout{};
        fe::vk::Pipeline       m_Pipeline{};

        Camera m_Camera{}; // temp

        bool m_IsWindowResized{}; // temp

        uint32_t m_CurrentFrame{};

        VulkanResourceManager m_VulkanResourceManager{ m_Context, m_ResourceManager };

        fe::vk::CommandPool m_CommandPool{};

        uint32_t m_ImageIndex{};

        size_t          m_MeshIndex{};
        GlobalSceneData m_SceneData{};
    };
} // namespace fe
