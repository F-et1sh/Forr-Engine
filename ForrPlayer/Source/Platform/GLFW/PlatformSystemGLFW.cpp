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

#include "Tools.hpp"

fe::PlatformSystemGLFW::~PlatformSystemGLFW() {
    glfwTerminate();
}

size_t fe::PlatformSystemGLFW::CreateWindow(const WindowDesc& desc) {
    std::unique_ptr<IWindow> window = std::make_unique<WindowGLFW>();
    window->Initialize(desc);

    m_WindowList.push_back(std::move(window));

    return m_WindowList.size() - 1;
}

FORR_NODISCARD std::vector<const char*> fe::PlatformSystemGLFW::getSurfaceRequiredExtensions() {
    uint32_t     extensions_count = 0;
    const char** extensions       = glfwGetRequiredInstanceExtensions(&extensions_count);
    return { extensions, extensions + extensions_count };
}

fe::PlatformSystemGLFW::PlatformSystemGLFW(const PlatformSystemDesc& desc) {
    GLFW_CHECK_RESULT(glfwInit())

    switch (desc.graphics_backend) {
        case GraphicsBackend::OpenGL:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            break;
        case GraphicsBackend::Vulkan:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
        default:
            break;
    }
}
