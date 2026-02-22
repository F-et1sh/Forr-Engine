/*===============================================

    Forr Engine

    File : RendererVulkanGL.hpp
    Role : Vulkan Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IRenderer.hpp"

#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Volk/volk.h"

#include "Graphics/GPUTypes.hpp"
#include "VKTypes.hpp"

namespace fe {
    class RendererVulkan : public IRenderer {
    public:
        RendererVulkan(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index);
        ~RendererVulkan();

        void ClearScreen(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) override;
        void SwapBuffers() override;

        void                                    Draw(MeshID index) override;
        FORR_FORCE_INLINE FORR_NODISCARD MeshID CreateTriangle() override { return 0; }; // temp

    private:
        VkShaderModule create_shader_module(const std::string& code);
        std::string    load_shader(const std::filesystem::path& path);
        void           record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index, VkRenderPass render_pass, std::vector<VkFramebuffer> swapchain_framebuffers, VkExtent2D swapchain_extent);

        uint32_t findMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
        void     createBuffer(VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);

        VkCommandBuffer beginSingleTimeCommands(VkCommandPool command_pool);
        void            endSingleTimeCommands(VkQueue graphics_queue, VkCommandPool command_pool, VkCommandBuffer command_buffer);
        void            copyBuffer(VkQueue graphics_queue, VkCommandPool command_pool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void createVertexBuffer(VkPhysicalDevice physical_device, VkQueue graphics_queue, VkCommandPool command_pool);
        void createIndexBuffer(VkPhysicalDevice physical_device, VkQueue graphics_queue, VkCommandPool command_pool);

    private:
        void CreateInstance();
        void CreateSurface();
        void ChoosePhysicalDevice();
        void ChooseQueueFamilies();
        void CreateLogicalDevice();
        void CreateSwapchain();
        void CreateSwapchainImageViews();
        void CreateRenderPass();
        void CreatePipelineLayout();
        void CreatePipeline();

    private:
        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        GLFWwindow* m_GLFWwindow = nullptr;

        fe::vk::Instance m_Instance{};
        fe::vk::Device   m_Device{};

        VkPhysicalDevice m_PhysicalDevice{};

        fe::vk::Swapchain        m_Swapchain{};
        fe::vk::Surface          m_Surface{};
        VkSurfaceFormatKHR       m_SurfaceFormat{};
        VkExtent2D               m_Extent{};
        std::vector<VkImageView> m_ImageViews{};

        std::vector<Vertex>   m_Vertices{};
        std::vector<uint32_t> m_Indices{};

        VkBuffer       m_VertexBuffer{};
        VkDeviceMemory m_VertexBufferMemory{};

        VkBuffer       m_IndexBuffer{};
        VkDeviceMemory m_IndexBufferMemory{};

        uint32_t m_GraphicsQueueFamilyIndex{};
        uint32_t m_PresentQueueFamilyIndex{};

        VkQueue m_GraphicsQueue{};
        VkQueue m_PresentQueue{};

        fe::vk::RenderPass m_RenderPass{};

        fe::vk::PipelineLayout m_PipelineLayout{};
        fe::vk::Pipeline       m_Pipeline{};
    };
} // namespace fe
