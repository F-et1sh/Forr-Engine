/*===============================================

    Forr Engine - Shared Module

    File : PlatformInterfaces.hpp
    Role : gives interfaces for Forr.Platform

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe::shared {
    struct WindowDesc {
        int width = 0;
        int height = 0;
    };
    
    class IWindow {
    public:
        IWindow()  = default;
        ~IWindow() = default;

        virtual void Release() = 0;
        virtual void Initialize(const WindowDesc& desc) = 0;

    protected:
        void* p_WindowHandle = nullptr;
    };
} // namespace fe::shared
