#include "pch.hpp"
#include "Application.hpp"

namespace fe {
    static entt::entity m_Object1{}; // temp
    static entt::entity m_Object2{}; // temp
    static entt::entity m_Light{};   // temp

    static std::unique_ptr<RenderSystem> m_RenderSystem{}; // temp
    static entt::registry                m_Registry{};     // temp

    static RenderPacket m_RenderPacket{}; // temp

} // namespace fe

fe::Application::Application(const ApplicationDesc& desc) {
    PATH.init(desc.args[0], true);

    this->InitializePlatformSystem(desc);
    this->InitializeResourceManager(desc);
    this->InitializePrimaryWindow(desc);
    this->InitializeRenderer(desc);

    size_t i = 0; // temp

    auto interesting_shader_vertex_ptr   = m_ResourceManager->ImportResource<resource::Shader>(PATH.getShadersPath() / L"Interesting" / L"shader.vert");
    auto interesting_shader_fragment_ptr = m_ResourceManager->ImportResource<resource::Shader>(PATH.getShadersPath() / L"Interesting" / L"shader.frag");

    resource::Material interesting_material{};
    interesting_material.vertex_shader_ptr   = interesting_shader_vertex_ptr;
    interesting_material.fragment_shader_ptr = interesting_shader_fragment_ptr;
    auto interesting_material_ptr            = m_ResourceManager->CreateResource<resource::Material>(std::move(interesting_material));

    m_ResourceManager->RunForEach<resource::Model>([&](fe::pointer<resource::Model> model_ptr, const resource::Model& model) { // temp
        switch (i) {
            case 1:
                m_Object1 = m_Registry.create();
                m_Registry.emplace<TransformComponent>(m_Object1, glm::translate(glm::mat4(1.0f), glm::vec3(50, 0, 0)));
                m_Registry.emplace<MeshComponent>(m_Object1, model_ptr, interesting_material_ptr);
                break;
            case 2:
                m_Object2 = m_Registry.create();
                m_Registry.emplace<TransformComponent>(m_Object2, glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));
                m_Registry.emplace<MeshComponent>(m_Object2, model_ptr);
                break;
        }
        i++;
    });

    m_Light = m_Registry.create();
    m_Registry.emplace<TransformComponent>(m_Light);
    m_Registry.emplace<LightComponent>(m_Light);

    m_Renderer->InitializeGPUResources();

    m_RenderSystem = std::make_unique<RenderSystem>(*m_ResourceManager, m_Registry, *m_Renderer, m_RenderPacket);

    auto& light0           = m_RenderPacket.lights.emplace_back();
    light0.position        = glm::vec4(0.0f, 3.0f, 0.0f, 1.0f); 
    light0.color_intensity = glm::vec4(1.0f, 0.6f, 0.3f, 10.0f);

    auto& light1           = m_RenderPacket.lights.emplace_back();
    light1.position        = glm::vec4(0.0f, -2.0f, 0.0f, 1.0f);
    light1.color_intensity = glm::vec4(0.2f, 0.6f, 1.0f, 10.0f);
}

size_t t{};

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
         float angle = static_cast<float>(t) * 0.02f;
        float radius = 15.0f;

        if (m_RenderPacket.lights.size() >= 2) {
            m_RenderPacket.lights[0].position.x = glm::sin(angle) * radius;
            m_RenderPacket.lights[0].position.y = 1.5f;
            m_RenderPacket.lights[0].position.z = glm::cos(angle) * radius;

            m_RenderPacket.lights[1].position.x = glm::sin(-angle) * radius;
            m_RenderPacket.lights[1].position.y = -0.5f;
            m_RenderPacket.lights[1].position.z = glm::cos(-angle) * radius;
        }

        m_RenderPacket.draw_commands.clear();
        m_RenderPacket.object_transforms.clear();

        m_Renderer->BeginFrame();
        m_RenderSystem->Update();
        m_Renderer->EndFrame(m_RenderPacket);

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
    paths.emplace_back(PATH.getEngineResourcesPath() / "Tatarstan-Flag.png");
    paths.emplace_back(PATH.getModelsPath() / "StatueOfLiberty/statue_of_liberty.glb");
    paths.emplace_back(PATH.getModelsPath() / "PirateRoom/PirateRoom.gltf");
    paths.emplace_back(PATH.getModelsPath() / "Suzanne/Suzanne.glb");

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
    m_Renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}
