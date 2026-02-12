#include "Forr/Application.hpp"

int main() {
    fe::Application app{};
    fe::ApplicationDesc desc{};

    app.Initialize(desc);
    app.Run();
    app.Release();
}