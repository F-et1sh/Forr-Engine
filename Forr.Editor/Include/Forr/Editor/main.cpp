/*===============================================

    Forr Engine - Editor Module

    File : main.cpp
    Role : entry point

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Engine/Window.hpp"

int main() {
    
    fe::engine::WindowDesc desc{};
    desc.window_width  = 1920;
    desc.window_height = 1080;

    fe::engine::Window window{};
    window.Initialize(desc);

    return 0;
}
