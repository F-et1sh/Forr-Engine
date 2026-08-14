/*===============================================

    Forr Engine

    File : IRenderer.hpp
    Role : Renderer interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string>
#include "Platform/IPlatformSystem.hpp"
#include "Core/types.hpp"

#include "ResourceManagement/ResourceManager.hpp"

#include "RenderGraph.hpp"

namespace fe {
    struct FORR_API RendererDesc {
        PlatformBackend platform_backend{};
        GraphicsBackend graphics_backend{};

        bool validation_enabled = true;

        std::string application_name{};
        WindowDesc  primary_window_desc{};

        RendererDesc()  = default;
        ~RendererDesc() = default;
    };

    struct DrawCommand {
        uint32_t index_count{};
        uint32_t index_offset{};
        uint32_t instance_index{};

        uint64_t sort_key{};

        // TODO : change this to fe::pointer<>
        GPUHandle<resource::Model::Mesh>    mesh_handle{};
        fe::pointer<fe::resource::Material> material_ptr{};

        DrawCommand()  = default;
        ~DrawCommand() = default;
    };

    // TODO : remove this
    struct RenderPacket {
        std::vector<glm::mat4>   object_transforms{};
        std::vector<DrawCommand> draw_commands{};
        std::vector<GPULight>    lights{};

        glm::mat4 projection_matrix{};
        glm::mat4 view_matrix{};

        RenderPacket()  = default;
        ~RenderPacket() = default;
    };

    // TODO : make it changeable dynamically and remove this
    constexpr inline static size_t MAX_CONCURRENT_FRAMES = 2;

    // if you want to add some variable here, use static method IRenderer::Create()
    // the member should be appended to the devired class, not here
    class FORR_API IRenderer {
    public:
        virtual ~IRenderer() = default;

        static std::unique_ptr<IRenderer> Create(const RendererDesc& desc,
                                                 IPlatformSystem&    platform_system,
                                                 size_t              primary_window_index,
                                                 ResourceManager&    resource_manager);

        virtual FORR_NODISCARD RenderGraphBindings CreateGPUResources(const RenderGraphCompileResult& compile_result) = 0;

        // create a buffer ( SSBO/UBO ) via its reflected data
        // @returns fe::ParameterID is a variable that can be used in fe::IRenderer::WriteBuffer() to
        //  bind and pass data to the buffer created in this function
        virtual FORR_NODISCARD ParameterID CreateParameter(const shader::ReflectedDescriptor& descriptor_layout) = 0;

        // write to SSBO or UBO
        virtual void WriteBuffer(ParameterID parameter_id, const std::vector<uint8_t>& data) = 0;

        virtual void BeginFrame()                                                   = 0;
        virtual void EndFrame(const render_graph::CommandList& render_command_list) = 0;

        // TODO : remove this. It should work other way
        virtual void InitializeGPUResources() = 0;
    };
} // namespace fe
