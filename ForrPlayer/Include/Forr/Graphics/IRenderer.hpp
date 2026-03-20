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
        std::string     application_name{};
        PlatformBackend platform_backend{};
        GraphicsBackend graphics_backend{};

        bool validation_enabled = true;

        RendererDesc()  = default;
        ~RendererDesc() = default;
    };

    struct DrawMeshCommand { // TODO : create DrawCommands.hpp
    public:
        fe::pointer<resource::Model> model_ptr{};
        uint32_t                     mesh_index = ~0; // ~0 means that renderer has to draw all meshes

        int i = 0; // temp

        glm::mat4 transform{};

        DrawMeshCommand()  = default;
        ~DrawMeshCommand() = default;
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

        virtual void BeginFrame()                  = 0;
        virtual void Draw(DrawMeshCommand command) = 0;
        virtual void EndFrame()                    = 0;

        virtual void InitializeGPUResources() = 0;
    };
} // namespace fe
