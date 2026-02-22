/*===============================================

    Forr Engine

    File : PlatformSystemGLFW.hpp
    Role : GLFW platform system implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Platform/IPlatformSystem.hpp"

namespace fe {
    class PlatformSystemGLFW : public IPlatformSystem {
    public:
        PlatformSystemGLFW(const PlatformSystemDesc& desc);
        ~PlatformSystemGLFW();

        // returns window index
        FORR_NODISCARD size_t CreateWindow(const WindowDesc& desc) override;

        FORR_NODISCARD size_t   getWindowCount() override { return m_WindowList.size(); }
        IWindow& getWindow(size_t index) override { return *m_WindowList[index]; }

        FORR_NODISCARD std::vector<const char*> getSurfaceRequiredExtensions() override;

    private:
        std::vector<std::unique_ptr<IWindow>> m_WindowList;
    };
} // namespace fe
