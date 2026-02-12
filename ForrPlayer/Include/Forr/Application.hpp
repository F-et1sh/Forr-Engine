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

namespace fe {
    struct FORR_API ApplicationDesc {
        PlatformSystemDesc platform_desc;
        WindowDesc         primary_window_desc;

        ApplicationDesc()  = default;
        ~ApplicationDesc() = default;
    };

    class FORR_API Application {
    public:
        Application() = default;
        Application(const ApplicationDesc& desc) { this->Initialize(desc); }

        ~Application() = default;

        void Initialize(const ApplicationDesc& desc);

        void Run();

    private:
        std::unique_ptr<IPlatformSystem> m_PlatformSystem;

        size_t   m_PrimaryWindowID = 0;
        IWindow* m_PrimaryWindow   = nullptr;
    };
} // namespace fe
