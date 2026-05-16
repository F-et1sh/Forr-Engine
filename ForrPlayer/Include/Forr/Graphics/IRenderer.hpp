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
    public:
        uint32_t instance_index{};

        uint32_t index_offset{};
        uint32_t index_count{};

        GPUHandle<resource::Model::Mesh> mesh_handle{};
        GPUHandle<resource::Material>    material_handle{};

        uint64_t sort_key{};

        glm::mat4 transform{};

        DrawCommand()  = default;
        ~DrawCommand() = default;
    };

    struct GlobalSceneData {
        glm::mat4 projection_matrix{};
        glm::mat4 view_matrix{};
        glm::mat4 model_matrices[32];

        GlobalSceneData()  = default;
        ~GlobalSceneData() = default;
    };

    // if you want to add some variable here, use static method IRenderer::Create()
    // the member should be assigned to the devired class, not here
    class FORR_API IRenderer {
    public:
        virtual ~IRenderer() = default;

        static std::unique_ptr<IRenderer> Create(const RendererDesc& desc,
                                                 IPlatformSystem&    platform_system,
                                                 size_t              primary_window_index,
                                                 ResourceManager&    resource_manager);

        virtual void SetClearColor(float red   = 1.0f,
                                   float green = 1.0f,
                                   float blue  = 1.0f,
                                   float alpha = 1.0f) = 0;

        virtual void BeginFrame()                     = 0;
        virtual void Draw(const DrawCommand& command) = 0;
        virtual void EndFrame()                       = 0;

        // TODO : remove this. It should work other way
        virtual void InitializeGPUResources() = 0;
    };
} // namespace fe
