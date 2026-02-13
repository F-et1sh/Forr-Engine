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
        IWindow()          = default;
        virtual ~IWindow() = default;

        FORR_CLASS_NONCOPYABLE(IWindow)

        virtual void Initialize(const WindowDesc& desc) = 0;

        FORR_FORCE_INLINE FORR_NODISCARD virtual bool IsOpen()     = 0;
        virtual void                                  PollEvents() = 0;

        FORR_FORCE_INLINE FORR_NODISCARD virtual void*    getNativeHandle() = 0;
        FORR_FORCE_INLINE FORR_NODISCARD virtual uint32_t getWidth()        = 0;
        FORR_FORCE_INLINE FORR_NODISCARD virtual uint32_t getHeight()       = 0;
        FORR_FORCE_INLINE FORR_NODISCARD virtual std::string getName()      = 0;
    };
} // namespace fe
