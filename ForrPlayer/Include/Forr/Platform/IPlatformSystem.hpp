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

#include "Forr/Core/attributes.hpp"
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

    using WindowID = size_t;

    class FORR_API IPlatformSystem {
    public:
        virtual ~IPlatformSystem() = default;

        FORR_NODISCARD static std::unique_ptr<IPlatformSystem> Create(const PlatformSystemDesc& desc, EventBus& event_bus);

        // returns window index
        FORR_NODISCARD virtual WindowID CreateWindow(const WindowDesc& desc) = 0;

        FORR_NODISCARD virtual WindowID                        getWindowCount()          = 0;
        virtual std::optional<std::reference_wrapper<IWindow>> getWindow(WindowID index) = 0;

        FORR_NODISCARD virtual std::vector<const char*> getSurfaceRequiredExtensions() = 0;
    };
} // namespace fe
