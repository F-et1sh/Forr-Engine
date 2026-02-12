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

void fe::PlatformSystemGLFW::Initialize(const PlatformSystemDesc& desc) {
    glfwInit(); // TODO : check return values
}
