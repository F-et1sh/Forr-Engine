/*===============================================

    Forr Engine

    File : IWindow.hpp
    Role : window interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/attributes.hpp"

namespace fe {
    struct FORR_API WindowDesc {
        uint32_t    width  = 0;
        uint32_t    height = 0;
        std::string name   = "Default Window";
    };

    class FORR_API IWindow {
    public:
        FORR_CLASS_NONCOPYABLE(IWindow)

        virtual void Initialize(const WindowDesc& desc) {}
        virtual void PollEvents() {}

        virtual void* getNativeHandle() {}

    protected:
        virtual void Release() {}
        virtual ~IWindow() = default;
    };
} // namespace fe
