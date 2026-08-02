/*===============================================

    Forr Engine

    File : RendererOpenGL.hpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <array>

#include "Graphics/IRenderer.hpp"
#include "Graphics/Camera.hpp"

#include "OpenGLResourceManager.hpp"

#include <GLFW/glfw3.h>

#include "Tools.hpp"

namespace fe {
    class RendererOpenGL : public IRenderer {
        constexpr inline static size_t MAX_CONCURRENT_FRAMES = 2;

    public:
        RendererOpenGL(const RendererDesc& desc,
                       IPlatformSystem&    platform_system,
                       size_t              primary_window_index,
                       ResourceManager&    resource_manager);
        ~RendererOpenGL();

        void SetClearColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) override;

        RenderGraphBindings CreateGPUResources(const RenderGraphCompileResult& compile_result) override;

        void BeginFrame() override;
        void EndFrame(const render_graph::CommandList& render_command_list) override;

        void InitializeGPUResources() override;

    private:
        void handleCommand(const render_graph::ImageBarrier& image_barrier);
        void handleCommand(const render_graph::BeginRenderPass& begin_render_pass);
        void handleCommand(const render_graph::EndRenderPass& end_render_pass);
        void handleCommand(const render_graph::DrawIndexed& draw_indices);
        void handleCommand(const render_graph::BindPipeline& bind_pipeline);

    private:
        struct FrameData {
            // Vulkan fence's analogue in OpenGL
            fe::gl::Sync       sync{};
            OpenGLShaderBuffer storage_buffer{};

            FrameData()  = default;
            ~FrameData() = default;
        };

        ResourceManager& m_ResourceManager;

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        GLFWwindow* m_GLFWwindow = nullptr;

        OpenGLResourceManager m_OpenGLResourceManager{ m_ResourceManager };

        Camera m_Camera{}; // temp

        GPUHandle<resource::Material>    m_CurrentMaterial{};
        GPUHandle<resource::Model::Mesh> m_CurrentMesh{};

        std::array<FrameData, MAX_CONCURRENT_FRAMES> m_FrameData{};

        // for render graph | temp
        std::unordered_map<uint64_t, gl::Framebuffer> m_FramebuffersCache{};

        uint32_t m_CurrentFrame{};
    };
} // namespace fe
