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

    struct GPULight {
        uint32_t type{};

        float range{};
        float inner_cone{};
        float outer_cone{};

        glm::vec4 position{};
        glm::vec4 direction{};
        glm::vec4 color_intensity{};
    };

    constexpr inline static uint64_t MAX_INSTANCES = 32;
    constexpr inline static uint64_t MAX_LIGHTS    = 32;

    struct SceneData {
        //uint32_t light_count{};

        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;

        glm::mat4 model_matrices[MAX_INSTANCES];

        //GPULight lights[MAX_LIGHTS];

        SceneData()  = default;
        ~SceneData() = default;
    };

    // if you want to add some variable here, use static method IRenderer::Create()
    // the member should be appended to the devired class, not here
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
