/*===============================================

    Forr Engine

    File : Application.cpp
    Role : main class / almost everything in this class should be in 'main.cpp' of the actual game/user

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Forr/Application.hpp"

// TODO : remove this. This should be in 'main.cpp' of the user
namespace fe {
    static entt::registry                            m_Registry{};        // temp
    static graphics::ParameterHandle                 m_ParameterHandle{}; // temp
    static std::vector<fe::pointer<resource::Model>> m_Models{};          // temp
    static std::vector<entt::entity>                 m_Entities{};        // temp
    static float                                     m_Time{};            // temp
} // namespace fe

fe::Application::Application(const ApplicationDesc& desc) {
    if (!desc.args.empty())
        PATH.init(std::string_view{ desc.args[0] }, true);
    else
        PATH.init("", true);

    this->InitializePlatformSystem(desc);
    this->InitializeResourceManager(desc);
    this->InitializePrimaryWindow(desc);
    this->InitializeRenderer(desc);

    m_Renderer->InitializeGPUResources();
}

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        m_Renderer->BeginFrame();

        auto& transform_component     = m_Registry.get<TransformComponent>(m_Entities[0]);
        transform_component.transform = glm::mat4(1.0f);
        transform_component.transform = glm::translate(transform_component.transform, glm::vec3(-1.5f, 0.5f, 0.0f));
        transform_component.transform = glm::rotate(transform_component.transform, m_Time, glm::vec3(0, 1, 0));
        transform_component.transform = glm::scale(transform_component.transform, glm::vec3(0.7f));

        auto& transform_component2     = m_Registry.get<TransformComponent>(m_Entities[1]);
        transform_component2.transform = glm::mat4(1.0f);
        transform_component2.transform = glm::translate(transform_component2.transform, glm::vec3(1.5f, 0.0f, 0.0f));
        transform_component2.transform = glm::rotate(transform_component2.transform, m_Time, glm::vec3(0, 1, 0));
        transform_component2.transform = glm::scale(transform_component2.transform, glm::vec3(2.0f));

        auto& transform_component3     = m_Registry.get<TransformComponent>(m_Entities[2]);
        transform_component3.transform = glm::mat4(1.0f);
        transform_component3.transform = glm::translate(transform_component3.transform, glm::vec3(0.0f, -0.5f, 0.0f));
        transform_component3.transform = glm::scale(transform_component3.transform, glm::vec3(5.0f));

        RenderGraphCollector<TransformComponent, ModelComponent, LightComponent> collector{ m_Registry };

        auto render_command_list = m_RenderGraph->Execute(collector.getRegistry());

        m_Renderer->EndFrame(render_command_list);

        m_PrimaryWindow->PollEvents();

        m_Time += 0.0025f;
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
    paths.emplace_back(PATH.getModelsPath() / "TatarSuzanne/TatarSuzanne.gltf");
    paths.emplace_back(PATH.getModelsPath() / "PirateRoom/PirateRoom.gltf");

    ResourceManagerDesc resource_manager_desc{};
    resource_manager_desc.graphics_backend = desc.graphics_backend;

    m_ResourceManager = std::make_unique<ResourceManager>(resource_manager_desc);
    m_ResourceManager->CreateDefaultResources();
    m_ResourceManager->SetupSceneResources(paths); // TODO : rewrite this

    m_Models.emplace_back() = m_ResourceManager->ImportResource<resource::Model>(PATH.getModelsPath() / "TatarSuzanne\\TatarSuzanne.gltf");
    m_Models.emplace_back() = m_ResourceManager->ImportResource<resource::Model>(PATH.getModelsPath() / "FlightHelmet\\FlightHelmet.gltf");
    m_Models.emplace_back() = m_ResourceManager->ImportResource<resource::Model>(PATH.getModelsPath() / "Plane\\plane.glb");

    for (auto& model : m_Models) {
        m_Entities.emplace_back() = m_Registry.create();
        m_Registry.emplace<ModelComponent>(m_Entities.back(), model);
        m_Registry.emplace<TransformComponent>(m_Entities.back());
    }

    LightComponent light_component{};

    //glm::mat4 proj = glm::perspective(glm::radians(45.0f),
    //                                  1920.0f / 1080.0f,
    //                                  0.1f,
    //                                  1000.0f);

    //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
    //                             glm::vec3(0.0f, 0.0f, 0.0f),
    //                             glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 proj = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 35.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    light_component.transform = proj * view;

    m_Entities.emplace_back() = m_Registry.create();
    m_Registry.emplace<LightComponent>(m_Entities.back(), light_component);
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

    m_RenderGraph                      = std::make_unique<RenderGraph>(*m_ResourceManager, *m_Renderer);
    auto shadow_pass_data_mapped       = m_RenderGraph->AddPass<ShadowPass>("Shadow Pass");            // TODO : make a storage for this mapped data
    auto forward_pass_data_mapped      = m_RenderGraph->AddPass<ForwardPass>("Forward Pass");          // TODO : make a storage for this mapped data
    auto post_process_pass_data_mapped = m_RenderGraph->AddPass<PostProcessPass>("Post Process Pass"); // TODO : make a storage for this mapped data

    auto create_command_list = std::move(m_RenderGraph->Compile());
    auto mapping_result      = std::move(m_Renderer->CreateRenderGraphResources(create_command_list));
    m_RenderGraph->SetupResourceBindings(mapping_result);
}
