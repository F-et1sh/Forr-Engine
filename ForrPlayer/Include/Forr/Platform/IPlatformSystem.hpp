/*===============================================

    Forr Engine

    File : IPlatformSystem.hpp
    Role : Platform system interface

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <vector>
#include <memory>

#include "Forr/Core/attributes.hpp"
#include "Forr/Core/pointer.hpp"
#include "Forr/Graphics/GPUTypes.hpp"
#include "Forr/Events/EventBus.hpp"

#include "IWindow.hpp"

namespace fe {
    enum class PlatformBackend : uint8_t {
        GLFW
    };

    struct PlatformSystemDesc {
        PlatformBackend platform_backend{};
        GraphicsBackend graphics_backend{};

        PlatformSystemDesc()  = default;
        ~PlatformSystemDesc() = default;
    };

    using WindowHandle = fe::pointer<std::unique_ptr<IWindow>>;

    // TODO : write more comments where
    class FORR_API IPlatformSystem {
    public:
        virtual ~IPlatformSystem() = default;

        // TODO : make the 'event_bus' optional
        FORR_NODISCARD static std::unique_ptr<IPlatformSystem> Create(const PlatformSystemDesc& desc, EventBus& event_bus);

        // returns window index
        FORR_NODISCARD virtual WindowHandle CreateWindow(const WindowDesc& desc) = 0;

        FORR_NODISCARD virtual size_t getWindowsCount()              = 0;
        virtual IWindow*              getWindow(WindowHandle window_ptr) = 0;

        FORR_NODISCARD virtual std::vector<const char*> getSurfaceRequiredExtensions() = 0;
    };
} // namespace fe
