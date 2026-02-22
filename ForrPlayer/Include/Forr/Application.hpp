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

namespace fe {
    struct FORR_API ApplicationDesc {
        char** argv;

        std::string application_name{};

        WindowDesc primary_window_desc{};

        GraphicsBackend graphics_backend{};
        PlatformBackend platform_backend{};

        ApplicationDesc()  = default;
        ~ApplicationDesc() = default;
    };

    class FORR_API Application {
    public:
        Application(const ApplicationDesc& desc);
        ~Application() = default;

        void Run();

    private:
        std::unique_ptr<IPlatformSystem> m_PlatformSystem;
        std::unique_ptr<IRenderer>       m_Renderer;

        size_t   m_PrimaryWindowID = 0;
        IWindow* m_PrimaryWindow   = nullptr;

        MeshID m_Triangle = 0; // temp
    };
} // namespace fe
