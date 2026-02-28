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
    m_Description = desc;

    int           monitor_count{};
    GLFWmonitor** monitors_raw = glfwGetMonitors(&monitor_count);

    if (m_Description.monitor_index == ~0) {
        m_GLFWmonitor = glfwGetPrimaryMonitor();
    }
    else if (m_Description.monitor_index >= 0 &&
             m_Description.monitor_index < monitor_count) {

        m_GLFWmonitor = monitors_raw[m_Description.monitor_index];
    }
    else {
        fe::logging::warning("Monitor index %i is out of range. Total monitor count is %i. Using primary ( default ) monitor", m_Description.monitor_index, monitor_count);
        
        m_GLFWmonitor = glfwGetPrimaryMonitor();
    }

    const GLFWvidmode* videomode{};
    videomode = glfwGetVideoMode(m_GLFWmonitor);

    if (m_Description.width == ~0) m_Description.width = videomode->width;
    if (m_Description.height == ~0) m_Description.height = videomode->height;

    GLFWmonitor* monitor = m_Description.is_fullscreen ? m_GLFWmonitor : nullptr;

    m_GLFWwindow = glfwCreateWindow(m_Description.width, m_Description.height, m_Description.name.c_str(), monitor, nullptr); // TODO : check GLFWwindow* share
    if (!m_GLFWwindow) {
        fe::logging::error("Failed to create GLFW window");
        return;
    }

    glfwSwapInterval(m_Description.vsync); // set VSync

    glfwSetWindowUserPointer(m_GLFWwindow, this);

    glfwSetWindowSizeCallback(m_GLFWwindow, windowSizeCallback);

    this->centralizeWindow();
}

bool fe::WindowGLFW::IsOpen() {
    return !glfwWindowShouldClose(m_GLFWwindow);
}

void fe::WindowGLFW::PollEvents() {
    glfwPollEvents();
}

void fe::WindowGLFW::setResolution(int width, int height) {
    glfwSetWindowSize(m_GLFWwindow, width, height);

    m_Description.width = width;
    m_Description.height = height;
}

void fe::WindowGLFW::centralizeWindow() const {
    assert(m_GLFWwindow);
    assert(m_GLFWmonitor);
    assert(m_Description.width != ~0);
    assert(m_Description.height != ~0);

    int x{};
    int y{};
    glfwGetMonitorPos(m_GLFWmonitor, &x, &y);

    const GLFWvidmode* videomode{};
    videomode = glfwGetVideoMode(m_GLFWmonitor);

    glfwSetWindowPos(m_GLFWwindow,
                     x + (videomode->width / 2) - (m_Description.width / 2),
                     y + (videomode->height / 2) - (m_Description.height / 2));
}

void fe::WindowGLFW::windowSizeCallback(GLFWwindow* window_glfw, int width, int height) {
    if (!window_glfw) return;
    
    WindowGLFW* window = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window_glfw));
    window->m_Description.width = width;
    window->m_Description.height = height;
}

fe::WindowGLFW::~WindowGLFW() {
    fe::logging::debug("Window \"%s\" destroyed", m_Description.name.c_str());
    glfwDestroyWindow(m_GLFWwindow);
}
