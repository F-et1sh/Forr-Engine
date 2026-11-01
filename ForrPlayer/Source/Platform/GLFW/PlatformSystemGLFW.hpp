/*===============================================

    Forr Engine

    File : PlatformSystemGLFW.hpp
    Role : GLFW platform system implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Platform/IPlatformSystem.hpp"

namespace fe {
    class PlatformSystemGLFW : public IPlatformSystem {
    public:
        PlatformSystemGLFW(const PlatformSystemDesc& desc, EventBus& event_bus);
        ~PlatformSystemGLFW() override;

        // returns window index
        FORR_NODISCARD WindowID CreateWindow(const WindowDesc& desc) override;

        FORR_NODISCARD WindowID getWindowCount() override { return m_WindowList.size(); }
        FORR_NODISCARD std::optional<std::reference_wrapper<IWindow>> getWindow(WindowID index) override {
            if (index >= m_WindowList.size() || m_WindowList[index] == nullptr) {
                return std::nullopt;
            }
            return *m_WindowList[index];
        }

        FORR_NODISCARD std::vector<const char*> getSurfaceRequiredExtensions() override;

    private:
        std::reference_wrapper<EventBus>      m_EventBus;
        std::vector<std::unique_ptr<IWindow>> m_WindowList{};
    };
} // namespace fe
