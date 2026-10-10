/*===============================================

    Forr Engine

    File : main.hpp
    Role : Application entry point

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include <memory>

#include "Forr/Platform/IPlatformSystem.hpp"
#include "Forr/Platform/IWindow.hpp"
#include "Forr/Graphics/IRenderer.hpp"
#include "Forr/Graphics/RenderGraph.hpp"
#include "Forr/ResourceManagement/ResourceManager.hpp"

int main(int argc, char* argv[]) {
    //std::unique_ptr<fe::IPlatformSystem> platform_system{};
    //std::unique_ptr<fe::IRenderer>       renderer{};
    //std::unique_ptr<fe::RenderGraph>     render_graph{};
    //std::unique_ptr<fe::ResourceManager> resource_manager{};

    //size_t       primary_windowID{};
    //fe::IWindow* primary_window{};

    //fe::GraphicsBackend selected_backend = fe::GraphicsBackend::OpenGL;

    //for (const char* arg : std::span(argv, argc)) {
    //    desc.args.emplace_back(arg);

    //    if (arg == "-OpenGL" || arg == "-opengl") {
    //        selected_backend = fe::GraphicsBackend::OpenGL;
    //    }
    //    else if (arg == "-Vulkan" || arg == "-vulkan") {
    //        selected_backend = fe::GraphicsBackend::Vulkan;
    //    }
    //}

    //desc.application_name                  = "ForrGame";
    //desc.primary_window_desc.width         = 1920;
    //desc.primary_window_desc.height        = 1080;
    //desc.primary_window_desc.is_fullscreen = false;
    //desc.primary_window_desc.name          = "Gmod Realism";
    //desc.primary_window_desc.vsync         = true;
    //desc.platform_backend                  = fe::PlatformBackend::GLFW;
    //desc.graphics_backend                  = selected_backend;

    return 0;
}
