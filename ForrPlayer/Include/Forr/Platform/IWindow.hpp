/*===============================================

    Forr Engine

    File : IWindow.hpp
    Role : window interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <string>
#include "Core/attributes.hpp"

namespace fe {
    struct FORR_API WindowDesc {
        int         width{ ~0 };
        int         height{ ~0 };
        std::string name{};
        int         monitor_index{ ~0 };
        int         vsync{};
        bool        is_fullscreen{};

        WindowDesc()  = default;
        ~WindowDesc() = default;
    };

    class FORR_API IWindow {
    public:
        IWindow()          = default;
        virtual ~IWindow() = default;

        FORR_CLASS_NONCOPYABLE(IWindow)

        virtual void Initialize(const WindowDesc& desc) = 0;

        FORR_NODISCARD virtual bool IsOpen()     = 0;
        virtual void                PollEvents() = 0;

        FORR_NODISCARD virtual void*       getNativeHandle() = 0;
        FORR_NODISCARD virtual int         getWidth()        = 0;
        FORR_NODISCARD virtual int         getHeight()       = 0;
        FORR_NODISCARD virtual std::string getName()         = 0;
        FORR_NODISCARD virtual int         getMonitorIndex() = 0;
        FORR_NODISCARD virtual int         getVSync()        = 0;
        FORR_NODISCARD virtual bool        getIsFullscreen() = 0;

        virtual void setResolution(int width, int height) = 0;
    };
} // namespace fe
