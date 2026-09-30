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

#include "Graphics/Slang/SlangParser.hpp"

namespace fe {
    class RendererOpenGL : public IRenderer {
    private:
        struct FrameData {
            // Vulkan fence's analogue in OpenGL
            fe::gl::Sync                       sync{};
            std::vector<fe::gl::Buffer>        buffers_to_destroy{};
            std::vector<fe::gl::ShaderProgram> shader_programs_to_destory{};

            FrameData() = default;
        };

    public:
        RendererOpenGL(const RendererDesc& desc,
                       IPlatformSystem&    platform_system,
                       size_t              primary_window_index,
                       ResourceManager&    resource_manager);
        ~RendererOpenGL();

        RenderGraphBindings CreateGPUResources(const RenderGraphCompileResult& compile_result) override;

        std::expected<ParameterID, ParameterCreationErrors> CreateParameter(const ParameterDesc& parameter_desc) override;
        void                                                BindParameter(ParameterID parameter_id) override;
        void                                                WriteParameter(ParameterID parameter_id, std::span<const std::byte> data) override;
        void                                                DestroyParameter(ParameterID parameter_id) override;

        FORR_NODISCARD std::expected<PipelineID, PipelineCreationErrors> CreatePipeline(const PipelineDesc& pipeline_desc) override;
        void                                                             BindPipeline(PipelineID pipeline_id) override;
        void                                                             DestroyPipeline(PipelineID pipeline_id) override;

        void BeginFrame() override;
        void EndFrame(const render_graph::CommandList& render_command_list) override;

        void InitializeGPUResources() override;

        GraphicsBackend GetCurrentGraphicsBackend() override { return GraphicsBackend::OpenGL; }

    private:
        void   bindPipeline(const OpenGLPipeline& pipeline);
        GLuint createShaderProgramRaw(const shader::ProgramSources& program_sources);

    private:
        void handleCommand(const render_graph::ImageBarrier& command);
        void handleCommand(const render_graph::BufferBarrier& command);
        void handleCommand(const render_graph::BeginRenderPass& command);
        void handleCommand(const render_graph::EndRenderPass& command);
        void handleCommand(const render_graph::DrawIndexed& command);
        void handleCommand(const render_graph::BindPipeline& command);
        void handleCommand(const render_graph::DrawModel& command);
        void handleCommand(const render_graph::BindBuffer& command);
        void handleCommand(const render_graph::WriteBuffer& command);

    private:
        ResourceManager& m_ResourceManager;

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        GLFWwindow* m_GLFWwindow = nullptr;

        std::array<FrameData, MAX_CONCURRENT_FRAMES> m_FrameData{};
        uint32_t                                     m_CurrentFrame{};

        // render targets' hash --> framebuffer
        std::unordered_map<uint64_t, gl::Framebuffer> m_FramebuffersCache{};

        // TODO : soon
        //
        //fe::typed_pointer_storage<OpenGLMesh, fe::graphics::MeshHandle>                     m_Meshes{};
        //fe::typed_pointer_storage<OpenGLTexture, fe::graphics::TextureHandle>               m_Textures{};
        //fe::typed_pointer_storage<OpenGLShaderParameterRing, fe::graphics::ParameterHandle> m_Parameters{};
        //fe::typed_pointer_storage<OpenGLPipeline, fe::graphics::PipelineHandle>             m_Pipelines{};

        fe::typed_pointer_storage<OpenGLShaderParameterRing, ParameterID> m_Parameters{};
        fe::typed_pointer_storage<OpenGLPipeline, PipelineID>             m_Pipelines{};
    };
} // namespace fe
