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
        VkShaderModule create_shader_module(const std::string& code, VkDevice device);
        std::string    load_shader(const std::filesystem::path& path);
        void           record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index, VkRenderPass render_pass, std::vector<VkFramebuffer> swapchain_framebuffers, VkExtent2D swapchain_extent, VkPipeline graphics_pipeline);

    private:
        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        GLFWwindow* m_GLFWwindow = nullptr;
    };
} // namespace fe
