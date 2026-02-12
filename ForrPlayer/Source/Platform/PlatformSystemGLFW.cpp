/*===============================================

    Forr Engine

    File : PlatformSystemGLFW.cpp
    Role : GLFW platform system implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "PlatformSystemGLFW.hpp"

#include <GLFW/glfw3.h>

void fe::PlatformSystemGLFW::Release() {
    glfwTerminate();
}

fe::IWindow& fe::PlatformSystemGLFW::CreateWindow(const WindowDesc& desc) {
    std::unique_ptr<IWindow> window = std::make_unique<WindowGLFW>(); // TODO : Create GLFW Window
    window->Initialize(desc);
    
    m_WindowList.push_back(std::move(window));

    return *m_WindowList.back();
}

void fe::PlatformSystemGLFW::Initialize(const PlatformSystemDesc& desc) {
    glfwInit(); // TODO : check return values
}
