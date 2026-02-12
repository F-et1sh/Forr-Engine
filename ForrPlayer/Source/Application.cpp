#include "pch.hpp"
#include "Application.hpp"

void fe::Application::Initialize(const fe::ApplicationDesc& desc) {

    m_PlatformSystem = IPlatformSystem::Create(desc.platform_desc);

    // create primary window
    m_PrimaryWindowID = m_PlatformSystem->CreateWindow(desc.primary_window_desc);
    m_PrimaryWindow   = &m_PlatformSystem->getWindow(m_PrimaryWindowID);
}

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        m_PrimaryWindow->PollEvents();
    }
}
