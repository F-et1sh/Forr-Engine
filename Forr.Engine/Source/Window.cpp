/*===============================================

    Forr Engine - Engine Module

    File : Window.cpp
    Role : window

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Window.hpp"
#include "Platform/Window.hpp"

struct fe::engine::Window::Impl {
    std::unique_ptr<fe::shared::IWindow> p_Window;
};

fe::engine::Window::Window() {
    p_Impl = new Impl();
}

fe::engine::Window::~Window() {
    delete p_Impl;
}

void fe::engine::Window::Release() {
}

void fe::engine::Window::Initialize(const WindowDesc& desc) {
    p_Impl->p_Window = std::make_unique<fe::platform::WindowGLFW>();

    fe::shared::WindowDesc _desc{};
    _desc.width = desc.window_height;
    _desc.height = desc.window_height;
    p_Impl->p_Window->Initialize(_desc);
}
