#include "pch.hpp"
#include "Application.hpp"

fe::Application::Application(const ApplicationDesc& desc) {
    m_PlatformSystem = IPlatformSystem::Create(desc.platform_desc);

    // create primary window
    m_PrimaryWindowID = m_PlatformSystem->CreateWindow(desc.primary_window_desc);
    m_PrimaryWindow   = &m_PlatformSystem->getWindow(m_PrimaryWindowID);

    m_Renderer = IRenderer::Create(desc.renderer_desc, *m_PlatformSystem, m_PrimaryWindowID);
}

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        m_Renderer->ClearScreen(0.5f, 0.5f, 0.5f, 1.0f);
        m_Renderer->SwapBuffers();
        m_PrimaryWindow->PollEvents();
    }
}
