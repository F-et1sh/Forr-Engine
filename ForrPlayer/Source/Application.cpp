#include "pch.hpp"
#include "Application.hpp"

fe::Application::Application(const ApplicationDesc& desc) {
    PATH.init(desc.args[0], true);

    this->InitializePlatformSystem(desc);
    this->InitializeResourceManager(desc);
    this->InitializePrimaryWindow(desc);
    this->InitializeRenderer(desc);

    size_t i = 0; // temp

    m_ResourceManager->RunForEach<resource::Model>([&](fe::pointer<resource::Model> model_ptr, const resource::Model& model) { // temp
        switch (i) {
            case 0:
                m_Object.mesh_component.model_ptr      = model_ptr;
                m_Object.mesh_component.mesh_id        = ~0;
                m_Object.transform_component.transform = glm::translate(glm::mat4(1.0f), glm::vec3(50, 0, 0));
                break;
            case 1:
                m_Object2.mesh_component.model_ptr      = model_ptr;
                m_Object2.mesh_component.mesh_id        = ~0;
                m_Object2.transform_component.transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
                break;
        }
        i++;
    });

    m_ResourceManager->RunForEach<resource::Shader>([&](fe::pointer<resource::Shader> shader_ptr, const resource::Shader& shader) {
        for (const auto& [name, property] : shader.properties) {
            fe::logging::info("%s -> %i", name.c_str(), property.offset);
        }
    });
}

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        m_Renderer->BeginFrame();

        { // temp
            DrawMeshCommand command{};
            command.model_ptr  = m_Object.mesh_component.model_ptr;
            command.mesh_index = m_Object.mesh_component.mesh_id;
            command.transform  = m_Object.transform_component.transform;

            m_Object.transform_component.transform = glm::rotate(m_Object.transform_component.transform, 0.01f, glm::vec3(0, 1, 0));

            m_Renderer->Draw(command);

            DrawMeshCommand command2{};
            command2.model_ptr  = m_Object2.mesh_component.model_ptr;
            command2.mesh_index = m_Object2.mesh_component.mesh_id;
            command2.transform  = m_Object2.transform_component.transform;

            m_Renderer->Draw(command2);
        }

        m_Renderer->EndFrame();

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
    //paths.emplace_back("Tatarstan-Flag.png");
    //paths.emplace_back("Models/StatueOfLiberty/statue_of_liberty.glb");
    //paths.emplace_back("Models/PirateRoom/PirateRoom.gltf");

    paths.emplace_back("Shaders/default.vert.spv");
    paths.emplace_back("Shaders/default.frag.spv");

    m_ResourceManager = std::make_unique<ResourceManager>();
    m_ResourceManager->SetupSceneResources(paths); // TODO : rewrite this
}

void fe::Application::InitializePrimaryWindow(const ApplicationDesc& desc) {
    m_PrimaryWindowID = m_PlatformSystem->CreateWindow(desc.primary_window_desc);
    m_PrimaryWindow   = &m_PlatformSystem->getWindow(m_PrimaryWindowID);
}

void fe::Application::InitializeRenderer(const ApplicationDesc& desc) {
    RendererDesc renderer_desc{};
    renderer_desc.platform_backend    = desc.platform_backend;
    renderer_desc.graphics_backend    = desc.graphics_backend;
    renderer_desc.application_name    = desc.application_name;
    renderer_desc.primary_window_desc = desc.primary_window_desc;
    renderer_desc.validation_enabled  = desc.validation_enabled;

    m_Renderer = IRenderer::Create(renderer_desc, *m_PlatformSystem, m_PrimaryWindowID, *m_ResourceManager);
    m_Renderer->InitializeGPUResources();

    m_Renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}
