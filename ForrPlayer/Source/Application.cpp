/*===============================================

    Forr Engine

    File : Application.cpp
    Role : main class / almost everything in this class should be in 'main.cpp' of the actual game/user

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Application.hpp"

// TODO : remove this. This should be in 'main.cpp' of the user
namespace fe {
    static entt::entity m_Object1{}; // temp
    static entt::entity m_Object2{}; // temp
    static entt::entity m_Light{};   // temp

    static std::unique_ptr<RenderSystem> m_RenderSystem{}; // temp
    static entt::registry                m_Registry{};     // temp

    static RenderPacket m_RenderPacket{}; // temp

    static fe::pointer<resource::ShaderFileData> m_PerFrameLayoutPtr; // temp

} // namespace fe

fe::Application::Application(const ApplicationDesc& desc) {
    PATH.init(desc.args[0], true);

    this->InitializePlatformSystem(desc);
    this->InitializeResourceManager(desc);
    this->InitializePrimaryWindow(desc);
    this->InitializeRenderer(desc);

    /*auto interesting_shader_vertex_ptr   = m_ResourceManager->ImportResource<resource::Shader>(PATH.getShadersPath() / L"Interesting" / L"shader.vert");
    auto interesting_shader_fragment_ptr = m_ResourceManager->ImportResource<resource::Shader>(PATH.getShadersPath() / L"Interesting" / L"shader.frag");

    resource::Material interesting_material{};
    interesting_material.vertex_shader_ptr   = interesting_shader_vertex_ptr;
    interesting_material.fragment_shader_ptr = interesting_shader_fragment_ptr;
    auto interesting_material_ptr            = m_ResourceManager->CreateResource<resource::Material>(std::move(interesting_material));*/

    m_ResourceManager->RunForEach<resource::Model>([&](fe::pointer<resource::Model> model_ptr, const resource::Model& model) { // temp
        m_Object1 = m_Registry.create();
        m_Registry.emplace<TransformComponent>(m_Object1, glm::translate(glm::mat4(1.0f), glm::vec3(50, 0, 0)));
        m_Registry.emplace<MeshComponent>(m_Object1, model_ptr /*, interesting_material_ptr*/);

        m_Object2 = m_Registry.create();
        m_Registry.emplace<TransformComponent>(m_Object2, glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));
        m_Registry.emplace<MeshComponent>(m_Object2, model_ptr);
    });

    m_Light = m_Registry.create();
    m_Registry.emplace<TransformComponent>(m_Light);
    m_Registry.emplace<LightComponent>(m_Light);

    m_Renderer->InitializeGPUResources();

    m_RenderSystem = std::make_unique<RenderSystem>(*m_ResourceManager, m_Registry, *m_Renderer, m_RenderPacket);
}

size_t t{};

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        float angle  = static_cast<float>(t) * 0.02f;
        float radius = 15.0f;

        auto& light_transform     = m_Registry.get<TransformComponent>(m_Light);
        light_transform.transform = glm::translate(glm::vec3(glm::sin(angle) * radius, 1.5f, glm::cos(angle) * radius));

        // reset
        m_RenderPacket.draw_commands.clear();
        m_RenderPacket.object_transforms.clear();
        m_RenderPacket.lights.clear();

        m_Renderer->BeginFrame();

        RenderGraphCollector<TransformComponent, MeshComponent, LightComponent> collector{ m_Registry };

        auto render_command_list = m_RenderGraph->Execute(collector.getRegistry());

        m_Renderer->EndFrame(render_command_list);

        m_PrimaryWindow->PollEvents();
        t++;
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
    //paths.emplace_back(PATH.getModelsPath() / "TatarSuzanne/TatarSuzanne.gltf");
    //paths.emplace_back(PATH.getModelsPath() / "PirateRoom/PirateRoom.gltf");

    ResourceManagerDesc resource_manager_desc{};
    resource_manager_desc.graphics_backend = desc.graphics_backend;

    m_ResourceManager = std::make_unique<ResourceManager>(resource_manager_desc);
    m_ResourceManager->CreateDefaultResources();
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

    m_PerFrameLayoutPtr = m_ResourceManager->ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Context" / "GlobalContext.slang"); // temp

    m_RenderGraph = std::make_unique<RenderGraph>(*m_ResourceManager);
    //auto shadow_pass_data_mapped  = m_RenderGraph->AddPass<ShadowPassData, ShadowPass>("Shadow Pass");    // TODO : make a storage for this mapped data
    auto forward_pass_data_mapped = m_RenderGraph->AddPass<ForwardPassData, ForwardPass>("Forward Pass"); // TODO : make a storage for this mapped data

    auto create_command_list = std::move(m_RenderGraph->Compile());
    auto mapping_result      = std::move(m_Renderer->CreateGPUResources(create_command_list));
    m_RenderGraph->SetupResourceBindings(mapping_result);
}
