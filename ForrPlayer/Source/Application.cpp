#include "pch.hpp"
#include "Application.hpp"

void fe::Application::Release() {
}

void fe::Application::Initialize(const fe::ApplicationDesc& desc) {

    m_PlatformSystem = IPlatformSystem::Create(desc.platform_desc);
}

void fe::Application::Run() {
}
