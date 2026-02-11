#include "Forr/Application.hpp"

int main() {
    fe::Application app{};

    fe::ApplicationDesc desc{};
    desc.window_width = 1920;
    desc.window_height = 1080;

    app.Initialize(desc);
    app.Run();
    app.Release();
}