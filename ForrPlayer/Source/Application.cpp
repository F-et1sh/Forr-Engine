#include "pch.hpp"
#include "Application.hpp"

fe::Application::Application(const ApplicationDesc& desc) {
    PATH.init(desc.args[0], true);

    this->InitializePlatformSystem(desc);
    this->InitializeResourceManager(desc);
    this->InitializePrimaryWindow(desc);
    this->InitializeRenderer(desc);

    m_ResourceManager->RunForEach<resource::Model>([&](fe::pointer<resource::Model> ptr) { // take the first pointer
        m_ModelPtr = ptr;
    });
}

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        m_Renderer->ClearScreen(0.5f, 0.5f, 0.5f, 1.0f);

        m_Renderer->Draw(m_ModelPtr); // temp

        m_Renderer->SwapBuffers();

        m_PrimaryWindow->PollEvents();
    }
}

void fe::Application::InitializePlatformSystem(const ApplicationDesc& desc) {
    PlatformSystemDesc platform_desc{};
    platform_desc.platform_backend = desc.platform_backend;
    platform_desc.graphics_backend = desc.graphics_backend;

    m_PlatformSystem = IPlatformSystem::Create(platform_desc);
}

void fe::Application::InitializeResourceManager(const ApplicationDesc& desc) {
    std::vector<std::filesystem::path> paths{}; // temp
    paths.emplace_back("Tatarstan-Flag.png");
    paths.emplace_back("Models/StatueOfLiberty/statue_of_liberty.glb");

    m_ResourceManager = std::make_unique<ResourceManager>();
    m_ResourceManager->SetupSceneResources(paths); // TODO : rewrite this
}

void fe::Application::InitializePrimaryWindow(const ApplicationDesc& desc) {
    m_PrimaryWindowID = m_PlatformSystem->CreateWindow(desc.primary_window_desc);
    m_PrimaryWindow   = &m_PlatformSystem->getWindow(m_PrimaryWindowID);
}

void fe::Application::InitializeRenderer(const ApplicationDesc& desc) {
    RendererDesc renderer_desc{};
    renderer_desc.platform_backend = desc.platform_backend;
    renderer_desc.graphics_backend = desc.graphics_backend;
    renderer_desc.application_name = desc.application_name;

    m_Renderer = IRenderer::Create(renderer_desc, *m_PlatformSystem, m_PrimaryWindowID, *m_ResourceManager);
    m_Renderer->InitializeGPUResources();
}
