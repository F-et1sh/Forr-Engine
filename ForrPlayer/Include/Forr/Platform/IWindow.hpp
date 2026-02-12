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
        uint32_t    width  = 0;
        uint32_t    height = 0;
        std::string name   = "Default Window";
    };

    class FORR_API IWindow {
    public:
        IWindow() = default;
        virtual ~IWindow() = default;
        
        FORR_CLASS_NONCOPYABLE(IWindow)

        virtual void Release() {}
        virtual void Initialize(const WindowDesc& desc) {}

        virtual void PollEvents() {}

        FORR_FORCE_INLINE virtual void* getNativeHandle() { return nullptr; }
    };
} // namespace fe
