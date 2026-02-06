/*===============================================

    Forr Engine - Platform Module

    File : Window.cpp
    Role : window

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Window.hpp"

#include <GLFW/glfw3.h>

struct fe::platform::Window::Impl {
    GLFWwindow* p_GLFWwindow = nullptr;
};

fe::platform::Window::Window() {
    p_Impl = new Impl();
}

fe::platform::Window::~Window() {
    delete p_Impl;
}
