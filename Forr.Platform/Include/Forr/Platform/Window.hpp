/*===============================================

    Forr Engine - Platform Module

    File : Window.hpp
    Role : gives implementations of fe::shared::IWindow

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include "Shared/PlatformInterfaces.hpp"

namespace fe::platform {
    class WindowGLFW : public fe::shared::IWindow {
    public:
        WindowGLFW();
        ~WindowGLFW();

        void Release()override;
        void Initialize(const fe::shared::WindowDesc& desc)override;

    private:
        struct Impl;
        Impl* p_Impl;
    };
} // namespace fe
