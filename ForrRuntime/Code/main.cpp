/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrRuntime

        ForrRuntime is a thin wrapper to execute games made with Forr-Engine

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include <Forr/Core/path.hpp>
#include <Forr/Events/EventBus.hpp>
#include <Forr/Events/Events.hpp>
#include <Forr/Platform/IPlatformSystem.hpp>
#include <Forr/Platform/IWindow.hpp>
#include <Forr/ResourceManagement/ResourceManager.hpp>
#include <Forr/Graphics/IRenderer.hpp>
#include <Forr/Graphics/RenderGraph.hpp>
#include <Forr/Graphics/DefaultRenderPasses.hpp>
#include <Forr/ECS/Scene.hpp>
#include <Forr/Core/context.hpp>

int main(int argc, char* argv[]) {
    // static variables

    static constexpr bool                is_editor{ false };
    static constexpr bool                validation_layers_enabled{ false };
    static constexpr fe::GraphicsBackend graphics_backend{ fe::GraphicsBackend::OpenGL };
    static constexpr fe::PlatformBackend platform_backend{ fe::PlatformBackend::GLFW };
    static constexpr fe::WindowDesc      primary_window_desc{
        .width         = 1920,
        .height        = 1080,
        .name          = "ForrRuntime - Example",
        .monitor_index = 0,
        .vsync         = true,
        .is_fullscreen = false
    };

    // path

    fe::PATH.init(argv[0], is_editor);

    // event

    std::unique_ptr<fe::EventBus> main_event_bus{};
    main_event_bus = std::make_unique<fe::EventBus>();

    // platform

    std::unique_ptr<fe::IPlatformSystem> platform_system{};

    fe::PlatformSystemDesc platform_desc{};
    platform_desc.platform_backend = platform_backend;
    platform_desc.graphics_backend = graphics_backend;

    platform_system = fe::IPlatformSystem::Create(platform_desc, *main_event_bus);

    // window

    fe::IWindow* primary_window{}; // TODO : change this raw pointer to something else
    fe::WindowID primary_window_id{};

    primary_window_id  = platform_system->CreateWindow(primary_window_desc);
    auto window_result = platform_system->getWindow(primary_window_id);
    if (!window_result.has_value()) {
        fe::logging::fatal("Failed to get the primary window.\nWindow index was %llu", primary_window_id);
        return 0;
    }
    primary_window = &window_result.value().get();

    // resource manager

    std::unique_ptr<fe::ResourceManager> resource_manager{};

    std::vector<std::filesystem::path> paths{}; // temp
    paths.emplace_back(fe::PATH.getModelsPath() / "TatarSuzanne/TatarSuzanne.gltf");
    paths.emplace_back(fe::PATH.getModelsPath() / "PirateRoom/PirateRoom.gltf");

    fe::ResourceManagerDesc resource_manager_desc{};
    resource_manager_desc.graphics_backend = graphics_backend;

    resource_manager = std::make_unique<fe::ResourceManager>(resource_manager_desc);
    resource_manager->CreateDefaultResources();
    resource_manager->SetupSceneResources(paths); // TODO : rewrite this

    // renderer

    std::unique_ptr<fe::IRenderer> renderer{};

    fe::RendererDesc renderer_desc{};
    renderer_desc.platform_backend    = platform_backend;
    renderer_desc.graphics_backend    = graphics_backend;
    renderer_desc.application_name    = primary_window_desc.name;
    renderer_desc.primary_window_desc = primary_window_desc;
    renderer_desc.validation_enabled  = validation_layers_enabled;

    renderer = fe::IRenderer::Create(renderer_desc, *platform_system, primary_window_id, *resource_manager);

    // render graph

    std::unique_ptr<fe::RenderGraph> render_graph{};

    render_graph = std::make_unique<fe::RenderGraph>(*resource_manager, *renderer);

    auto forward_pass_data_mapped = render_graph->AddPass<fe::ForwardPass>("Forward Pass");

    auto create_command_list = render_graph->Compile();
    auto mapping_result      = renderer->CreateRenderGraphResources(create_command_list);
    render_graph->SetupResourceBindings(mapping_result);

    // registry

    entt::registry registry{};

    fe::context engine_context{};
    engine_context.emplace(*main_event_bus, fe::make_hash("main"));
    engine_context.emplace(*platform_system, fe::make_hash("main"));
    engine_context.emplace(*resource_manager, fe::make_hash("main"));
    engine_context.emplace(*renderer, fe::make_hash("main"));
    engine_context.emplace(*render_graph, fe::make_hash("main"));
    engine_context.emplace(registry, fe::make_hash("main"));

    struct SomeSystem {
        using DataT = fe::EmptySystemDataT;

        static void Initialize(fe::context&, DataT&) {};
        static void Update(fe::context&, DataT&) {};
        static void Release(DataT&) {};
    };

    fe::Scene scene{};
    scene.AddSystem<SomeSystem>("SomeSystem");

    // main loop

    while (primary_window->IsOpen()) {
        primary_window->PollEvents();

        main_event_bus->DispatchAll();
        main_event_bus->Clear();

        renderer->BeginFrame();

        fe::RenderGraphCollector<fe::component::Transform,
                                 fe::component::Camera,
                                 fe::component::ViewProjection,
                                 fe::component::LocalToWorld,
                                 fe::component::Model,
                                 fe::component::Light>
            collector{ registry };

        auto render_command_list = render_graph->Execute(collector.getRegistry());

        renderer->EndFrame(render_command_list);
    }

    return 0;
}
