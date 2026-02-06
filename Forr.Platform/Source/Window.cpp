/*===============================================

    Forr Engine - Platform Module

    File : Window.cpp
    Role : gives implementations of fe::shared::IWindow

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Window.hpp"

#include "Core/logging.hpp"

#include <GLFW/glfw3.h>

struct fe::platform::WindowGLFW::Impl {
    GLFWwindow* p_GLFWwindow = nullptr;
};

fe::platform::WindowGLFW::WindowGLFW() {
    p_Impl = new Impl();

    if (!glfwInit())
        fe::core::logging::fatal("Failed to initialize GLFW");
}

fe::platform::WindowGLFW::~WindowGLFW() {
    delete p_Impl;
}

void fe::platform::WindowGLFW::Release() {
    glfwTerminate(); // TODO : Move this
}

void fe::platform::WindowGLFW::Initialize(const fe::shared::WindowDesc& desc) {
    p_Impl->p_GLFWwindow = glfwCreateWindow(desc.width, desc.height, "Forr-Engine", 0, 0);
    if (!p_Impl->p_GLFWwindow) {
        glfwTerminate();
        fe::core::logging::fatal("Failed to create GLFW window");
    }

    this->p_WindowHandle = p_Impl->p_GLFWwindow;
}
