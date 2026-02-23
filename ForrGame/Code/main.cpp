#include "Forr/Application.hpp"
#include "Forr/Core/path.hpp"

int main(int argc, char* argv[]) {
    fe::ApplicationDesc desc{};
    for (int i = 0; i < argc; i++) desc.args.emplace_back(argv[i]);
    desc.application_name           = "ForrGame";
    desc.primary_window_desc.width  = 800;
    desc.primary_window_desc.height = 600;
    desc.primary_window_desc.is_fullscreen = false;
    desc.primary_window_desc.name   = "Gmod Realism";
    desc.platform_backend           = fe::PlatformBackend::GLFW;
    desc.graphics_backend           = fe::GraphicsBackend::Vulkan;

    fe::Application app{ desc };
    app.Run();
}
