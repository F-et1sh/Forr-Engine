/*===============================================

    Forr Engine

    File : PlatformSystemGLFW.hpp
    Role : GLFW platform system implementation

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include "Forr/Platform/IPlatformSystem.hpp"

namespace fe {
    class PlatformSystemGLFW : public IPlatformSystem {
    public:
        PlatformSystemGLFW(const PlatformSystemDesc& desc, EventBus& event_bus);
        ~PlatformSystemGLFW() override;

        // returns window index
        FORR_NODISCARD WindowHandle CreateWindow(const WindowDesc& desc) override;

        FORR_NODISCARD size_t   getWindowsCount() override { return m_WindowList.size(); }
        FORR_NODISCARD IWindow* getWindow(WindowHandle window_ptr) override {
            return m_WindowList.get(window_ptr)->get();
        }

        FORR_NODISCARD std::vector<const char*> getSurfaceRequiredExtensions() override;

    private:
        std::reference_wrapper<EventBus>                    m_EventBus;
        fe::typed_pointer_storage<std::unique_ptr<IWindow>> m_WindowList{};
    };
} // namespace fe
