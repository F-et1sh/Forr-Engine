/*===============================================

    Forr Engine

    File : WindowGLFW.cpp
    Role : GLFW window implemetation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "WindowGLFW.hpp"

void fe::WindowGLFW::Initialize(const WindowDesc& desc) {
    m_GLFWwindow = glfwCreateWindow(desc.width, desc.height, desc.name.c_str(), nullptr, nullptr); // TODO : check nullptrs
    if (!m_GLFWwindow) {
        fe::logging::error("Failed to create GLFW window");
        return;
    }
    m_Description = desc;
}

bool fe::WindowGLFW::IsOpen() {
    return !glfwWindowShouldClose(m_GLFWwindow);
}

void fe::WindowGLFW::PollEvents() {
    glfwPollEvents();
}

fe::WindowGLFW::~WindowGLFW() {
    fe::logging::debug("Window \"%s\" destroyed", m_Description.name.c_str());
    glfwDestroyWindow(m_GLFWwindow);
}
