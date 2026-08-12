/*===============================================

    Forr Engine

    File : Application.hpp
    Role : main class

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include <vector>

#include "Platform/IPlatformSystem.hpp"
#include "Graphics/IRenderer.hpp"
#include "Graphics/RenderGraph.hpp"
#include "ResourceManagement/ResourceManager.hpp"

#include "Graphics/DefaultRenderPasses.hpp"

#include "ECS/Components.hpp" // temp
#include "ECS/Systems/RenderSystem.hpp" // temp

namespace fe {
    struct FORR_API ApplicationDesc {
        bool validation_enabled = true;

        GraphicsBackend graphics_backend{};
        PlatformBackend platform_backend{};

        std::vector<const char*> args;

        std::string application_name{};
        WindowDesc  primary_window_desc{};

        ApplicationDesc()  = default;
        ~ApplicationDesc() = default;
    };

    class FORR_API Application { // TODO : move all logic of 'fe::Application' to 'main.cpp' of the game
    public:
        Application(const ApplicationDesc& desc);
        ~Application() = default;

        void Run();

    private:
        void InitializePlatformSystem(const ApplicationDesc& desc);
        void InitializeResourceManager(const ApplicationDesc& desc);
        void InitializePrimaryWindow(const ApplicationDesc& desc);
        void InitializeRenderer(const ApplicationDesc& desc);

    private:
        std::unique_ptr<IPlatformSystem> m_PlatformSystem{};
        std::unique_ptr<IRenderer>       m_Renderer{};
        std::unique_ptr<RenderGraph>     m_RenderGraph{};
        std::unique_ptr<ResourceManager> m_ResourceManager{};

        size_t   m_PrimaryWindowID{};
        IWindow* m_PrimaryWindow{};
    };
} // namespace fe
