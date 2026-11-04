/*===============================================

    Forr Engine

    File : Application.hpp
    Role : main class / almost everything in this class should be in 'main.cpp' of the actual game/user

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include <vector>

#include "Forr/Platform/IPlatformSystem.hpp"
#include "Forr/Graphics/IRenderer.hpp"
#include "Forr/Graphics/RenderGraph.hpp"
#include "Forr/ResourceManagement/ResourceManager.hpp"
#include "Forr/Graphics/DefaultRenderPasses.hpp"
#include "Forr/ECS/Components.hpp"
#include "Forr/ECS/Systems/RenderSystem.hpp"
#include "Forr/Events/EventBus.hpp"
#include "Forr/Events/Events.hpp"
#include "Forr/Core/type_registry.hpp"
#include "Forr/Core/context.hpp"
#include "Forr/Input/InputSystem.hpp"

namespace fe {
    struct ApplicationDesc {
        bool validation_enabled = true;

        GraphicsBackend graphics_backend{};
        PlatformBackend platform_backend{};

        std::vector<const char*> args{};

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
        std::unique_ptr<EventBus>        m_MainEventBus{};
        std::unique_ptr<InputSystem>     m_InputSystem{};

        fe::IWindow* m_PrimaryWindow{}; // TODO : change this raw pointer to something else
        WindowID     m_PrimaryWindowID{};
    };
} // namespace fe
