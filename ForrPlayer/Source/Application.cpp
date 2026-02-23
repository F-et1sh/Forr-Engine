#include "pch.hpp"
#include "Application.hpp"

fe::Application::Application(const ApplicationDesc& desc) {
    PATH.init(desc.args[0], true);

    PlatformSystemDesc platform_desc{};
    platform_desc.platform_backend = desc.platform_backend;
    platform_desc.graphics_backend = desc.graphics_backend;

    m_PlatformSystem = IPlatformSystem::Create(platform_desc);

    // create primary window
    m_PrimaryWindowID = m_PlatformSystem->CreateWindow(desc.primary_window_desc);
    m_PrimaryWindow   = &m_PlatformSystem->getWindow(m_PrimaryWindowID);

    RendererDesc renderer_desc{};
    renderer_desc.platform_backend = desc.platform_backend;
    renderer_desc.graphics_backend = desc.graphics_backend;
    renderer_desc.application_name = desc.application_name;

    m_Renderer = IRenderer::Create(renderer_desc, *m_PlatformSystem, m_PrimaryWindowID);

    m_Triangle = m_Renderer->CreateTriangle();
}

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        m_Renderer->ClearScreen(0.5f, 0.5f, 0.5f, 1.0f);

        m_Renderer->Draw(m_Triangle);

        m_Renderer->SwapBuffers();

        m_PrimaryWindow->PollEvents();
    }
}
