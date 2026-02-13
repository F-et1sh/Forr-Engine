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

        void*    getNativeHandle() override { return static_cast<void*>(m_GLFWwindow); }
        uint32_t getWidth() override { return m_Desc.width; }
        uint32_t getHeight() override { return m_Desc.height; }
        std::string getName() override { return m_Desc.name; }

    private:
        GLFWwindow* m_GLFWwindow = nullptr;

        WindowDesc m_Desc;
    };
} // namespace fe
