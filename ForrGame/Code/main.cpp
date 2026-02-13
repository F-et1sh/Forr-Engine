#include "Forr/Application.hpp"

int main() {
    fe::ApplicationDesc desc{};
    desc.primary_window_desc.width  = 1920;
    desc.primary_window_desc.height = 1080;
    desc.primary_window_desc.name   = "Gmod Realism";

    fe::Application app{desc};
    app.Run();
}
