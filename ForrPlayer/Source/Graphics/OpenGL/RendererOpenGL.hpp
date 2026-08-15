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
    public:
        RendererOpenGL(const RendererDesc& desc,
                       IPlatformSystem&    platform_system,
                       size_t              primary_window_index,
                       ResourceManager&    resource_manager);
        ~RendererOpenGL();

        RenderGraphBindings CreateGPUResources(const RenderGraphCompileResult& compile_result) override;

        ParameterID CreateParameter(const shader::ReflectedDescriptor& descriptor_layout) override;

        void BindBuffer(ParameterID parameter_id) override;
        void WriteBuffer(ParameterID parameter_id, std::span<const std::byte> data) override;

        void BeginFrame() override;
        void EndFrame(const render_graph::CommandList& render_command_list) override;

        void InitializeGPUResources() override;

    private:
        static void bindPipeline(const OpenGLPipeline& pipeline);

    private:
        void handleCommand(const render_graph::ImageBarrier& command);
        void handleCommand(const render_graph::BufferBarrier& command);
        void handleCommand(const render_graph::BeginRenderPass& command);
        void handleCommand(const render_graph::EndRenderPass& command);
        void handleCommand(const render_graph::DrawIndexed& command);
        void handleCommand(const render_graph::BindShaderProgram& command);
        void handleCommand(const render_graph::BindMaterial& command);
        void handleCommand(const render_graph::BindModel& command);
        void handleCommand(const render_graph::BindBuffer& command);
        void handleCommand(const render_graph::WriteBuffer& command);

    private:
        struct FrameData {
            // Vulkan fence's analogue in OpenGL
            fe::gl::Sync           sync{};
            OpenGLShaderDescriptor storage_buffer{};

            FrameData()  = default;
            ~FrameData() = default;
        };

        ResourceManager& m_ResourceManager;

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        GLFWwindow* m_GLFWwindow = nullptr;

        OpenGLResourceManager m_OpenGLResourceManager{ m_ResourceManager };

        Camera m_Camera{}; // temp

        // TODO : remove this
        GPUHandle<resource::Material>    m_CurrentMaterial{};
        GPUHandle<resource::Model::Mesh> m_CurrentMesh{};

        fe::pointer<resource::ShaderProgram> m_BoundShaderProgramPtr{};

        std::array<FrameData, MAX_CONCURRENT_FRAMES> m_FrameData{};

        // for render graph | temp
        std::unordered_map<uint64_t, gl::Framebuffer> m_FramebuffersCache{};

        uint32_t m_CurrentFrame{};
    };
} // namespace fe
