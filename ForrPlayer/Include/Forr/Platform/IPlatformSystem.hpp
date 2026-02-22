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
    enum class FORR_API GraphicsBackend {
        OpenGL,
        Vulkan
    };

    enum class FORR_API PlatformBackend {
        GLFW
    };

    struct FORR_API PlatformSystemDesc {
        PlatformBackend platform_backend{};
        GraphicsBackend graphics_backend{};

        PlatformSystemDesc()  = default;
        ~PlatformSystemDesc() = default;
    };

    class FORR_NODISCARD FORR_API IPlatformSystem {
    public:
        virtual ~IPlatformSystem() = default;

        FORR_NODISCARD static std::unique_ptr<IPlatformSystem> Create(const PlatformSystemDesc& desc);

        // returns window index
        FORR_NODISCARD virtual size_t CreateWindow(const WindowDesc& desc) = 0;

        FORR_NODISCARD virtual size_t getWindowCount()        = 0;
        virtual IWindow&              getWindow(size_t index) = 0;

        FORR_NODISCARD virtual std::vector<const char*> getSurfaceRequiredExtensions() = 0;
    };
} // namespace fe
