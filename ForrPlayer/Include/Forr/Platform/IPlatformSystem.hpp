/*===============================================

    Forr Engine

    File : IPlatformSystem.hpp
    Role : Platform system interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <memory>
#include "Core/attributes.hpp"

#include "IWindow.hpp"

namespace fe {
    enum class FORR_API PlatformBackend {
        GLFW
    };

    struct FORR_API PlatformSystemDesc {
        PlatformBackend backend;
    };

    class FORR_API IPlatformSystem {
    public:
        virtual ~IPlatformSystem() = default;

        virtual void Release() {}

        static std::unique_ptr<IPlatformSystem> Create(const PlatformSystemDesc& desc);

        virtual IWindow& CreateWindow(const WindowDesc& desc) = 0;

        FORR_FORCE_INLINE virtual size_t   getWindowCount() = 0;
        FORR_FORCE_INLINE virtual IWindow& getWindow(size_t index) = 0;

    protected:
        // protected, because PlatformSystem initializes in fe::IPlatformSystem::Create(...)
        virtual void Initialize(const PlatformSystemDesc& desc) {}
    };
} // namespace fe
