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
    static entt::registry m_Registry{};    // temp
    static ParameterID    m_ParameterID{}; // temp
} // namespace fe

fe::Application::Application(const ApplicationDesc& desc) {
    PATH.init(desc.args[0], true);

    this->InitializePlatformSystem(desc);
    this->InitializeResourceManager(desc);
    this->InitializePrimaryWindow(desc);
    this->InitializeRenderer(desc);

    m_Renderer->InitializeGPUResources();
}

size_t t{};

void fe::Application::Run() {
    while (m_PrimaryWindow->IsOpen()) {
        float angle  = static_cast<float>(t) * 0.02f;
        float radius = 15.0f;

        m_Renderer->BeginFrame();

        RenderGraphCollector<TransformComponent, LightComponent> collector{ m_Registry };
        auto                                                     render_command_list = m_RenderGraph->Execute(collector.getRegistry());

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
    paths.emplace_back(PATH.getModelsPath() / "TatarSuzanne/TatarSuzanne.gltf");
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

    { // temp
        auto        ptr                = m_ResourceManager->ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Context" / "GlobalContext.slang");
        const auto& shader_file_data   = *m_ResourceManager->GetResource<resource::ShaderFileData>(ptr);
        const auto& descriptors_layout = *m_ResourceManager->GetResource(shader_file_data.descriptors_layout_ptr.value());

        auto it = std::ranges::find_if(descriptors_layout.reflected_layout.descriptors, [](const shader::ReflectedDescriptor& descriptor) -> bool {
            return descriptor.name == fe::hashed_string{ "model_matrices" };
        });

        if (it == descriptors_layout.reflected_layout.descriptors.end()) {
            fe::logging::error("Failed to find model_matrices");
        }
        else {
            const auto& descriptor = *it;
            m_ParameterID          = m_Renderer->CreateParameter(descriptor);
        }
    }

    m_RenderGraph = std::make_unique<RenderGraph>(*m_ResourceManager);
    //auto shadow_pass_data_mapped  = m_RenderGraph->AddPass<ShadowPassData, ShadowPass>("Shadow Pass");  // TODO : make a storage for this mapped data
    auto forward_pass_data_mapped = m_RenderGraph->AddPass<ForwardPassData, ForwardPass>("Forward Pass"); // TODO : make a storage for this mapped data

    auto create_command_list = std::move(m_RenderGraph->Compile());
    auto mapping_result      = std::move(m_Renderer->CreateGPUResources(create_command_list));
    m_RenderGraph->SetupResourceBindings(mapping_result);
}
