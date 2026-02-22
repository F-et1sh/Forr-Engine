/*===============================================

    Forr Engine

    File : WindowGLFW.hpp
    Role : GLFW window implemetation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Platform/IWindow.hpp"

#include <GLFW/glfw3.h>

namespace fe {
    class WindowGLFW : public IWindow {
    public:
        WindowGLFW() = default;
        ~WindowGLFW();

        void Initialize(const WindowDesc& desc) override;

        FORR_NODISCARD bool IsOpen() override;
        void                PollEvents() override;

        FORR_NODISCARD void*    getNativeHandle() override { return static_cast<void*>(m_GLFWwindow); }
        FORR_NODISCARD uint32_t getWidth() override { return m_Description.width; }
        FORR_NODISCARD uint32_t getHeight() override { return m_Description.height; }
        FORR_NODISCARD std::string getName() override { return m_Description.name; }

    private:
        GLFWwindow* m_GLFWwindow = nullptr;

        WindowDesc m_Description;
    };
} // namespace fe
