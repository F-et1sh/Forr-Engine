#include "Forr/Application.hpp"
#include "Forr/Core/path.hpp"

int main(int argc, char* argv[]) {
    fe::ApplicationDesc desc{};
    desc.argv                       = argv;
    desc.primary_window_desc.width  = 1920;
    desc.primary_window_desc.height = 1080;
    desc.primary_window_desc.name   = "Gmod Realism";
    desc.platform_backend           = fe::PlatformBackend::GLFW;
    desc.graphics_backend           = fe::GraphicsBackend::Vulkan;

    fe::Application app{ desc };
    app.Run();
}
