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

        bool IsOpen() override;
        void PollEvents() override;

        FORR_NODISCARD void* getNativeHandle() { return static_cast<void*>(m_GLFWwindow); }

    private:
        GLFWwindow* m_GLFWwindow = nullptr;

        WindowDesc m_Desc;
    };
} // namespace fe
