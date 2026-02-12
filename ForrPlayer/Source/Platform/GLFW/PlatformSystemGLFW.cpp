/*===============================================

    Forr Engine

    File : PlatformSystemGLFW.cpp
    Role : GLFW platform system implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "PlatformSystemGLFW.hpp"

#include "WindowGLFW.hpp"

#define CHECK_GLFW(func)                                                          \
    {                                                                             \
        int result = func;                                                        \
        if (result != GLFW_TRUE) {                                                \
            fe::logging::error("Failed to call %s in file %s", ##func, __FILE__); \
        }                                                                         \
    }

fe::PlatformSystemGLFW::~PlatformSystemGLFW() {
    glfwTerminate();
}

size_t fe::PlatformSystemGLFW::CreateWindow(const WindowDesc& desc) {
    std::unique_ptr<IWindow> window = std::make_unique<WindowGLFW>(); // TODO : Create GLFW Window
    window->Initialize(desc);

    m_WindowList.push_back(std::move(window));

    return m_WindowList.size() - 1;
}

void fe::PlatformSystemGLFW::Initialize(const PlatformSystemDesc& desc) {
    CHECK_GLFW(glfwInit())
}
