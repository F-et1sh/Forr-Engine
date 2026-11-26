/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrGame

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
#include <Forr/Core/dll_api.hpp>

namespace game {
    namespace components {
        struct MyComponent {
            int a{};
        };

        struct MyComponent2 {
            int a{};
        };

#define GAME_COMPONENTS_LIST(X) \
    X(MyComponent)              \
    X(MyComponent2)

#define GAME_GENERATE_COMPONENTS_TUPLE(COMPONENT_NAME) COMPONENT_NAME,

        using AllComponents = fe::component::detail::TupleRemoveLast<GAME_COMPONENTS_LIST(GAME_GENERATE_COMPONENTS_TUPLE) void>;

#undef GAME_GENERATE_COMPONENTS_TUPLE
    } // namespace components
} // namespace game

struct ForwardPassData { // everything is temp
    fe::graphics::PipelineHandle pipeline_id{};

    fe::pointer<fe::resource::Material> default_material_ptr{};

    fe::graphics::ParameterHandle light_projection_parameter{};
    fe::graphics::ParameterHandle model_matrices_parameter_id{};
    std::vector<glm::mat4>        data{};

    struct alignas(16) PBRMaterialData {
        uint64_t  base_color_texture_handle{};
        glm::vec4 base_color_factor{};
    };
    fe::graphics::ParameterHandle materials_parameter_id{};
    std::vector<std::byte>        materials_data{};

    struct alignas(16) GlobalData {
        glm::mat4 view{};
        glm::mat4 projection{};

        GlobalData() = default;
    };
    fe::graphics::ParameterHandle global_data_parameter_id{};
    std::vector<std::byte>        global_data_as_bytes{};
};

struct ForwardPass {
    static void Setup(fe::RenderGraphBuilder& builder, ForwardPassData& pass_data) { // setup can be called twice
        builder.writeToScreen(true);
        builder.createImage(fe::render_graph::ImageDesc{
            .handle     = fe::make_hash("RenderTarget"),
            .type       = fe::render_graph::ImageType::IMAGE_TYPE_2D,
            .format     = fe::render_graph::Format::RGBA8_UNORM,
            .extent     = glm::ivec3{ 1920, 1080, 1 },
            .mip_levels = 1,
            .usage      = fe::render_graph::ImageUsageBits::RENDER_TARGET });

        builder.createImage(fe::render_graph::ImageDesc{
            .handle     = fe::make_hash("Depth"),
            .type       = fe::render_graph::ImageType::IMAGE_TYPE_2D,
            .format     = fe::render_graph::Format::D32_SFLOAT,
            .extent     = glm::ivec3{ 1920, 1080, 1 },
            .mip_levels = 1,
            .usage      = fe::render_graph::ImageUsageBits::DEPTH_STENCIL });

        builder.writeImage(fe::make_hash("RenderTarget"), fe::ResourceState::RENDER_TARGET);
        builder.writeImage(fe::make_hash("Depth"), fe::ResourceState::DEPTH_WRITE);
        //builder.readImage(fe::make_hash("ShadowMap"), fe::ResourceState::SHADER_READ_ONLY);

        fe::pointer<fe::resource::ShaderFileData> shader_file_data_ptr = builder.resource_manager.ImportResource<fe::resource::ShaderFileData>(fe::PATH.getShadersPath() / "Default\\PBRMaterial\\PBRMaterial.slang");
        fe::resource::ShaderFileData&             shader_file_data     = *builder.resource_manager.GetResource(shader_file_data_ptr);

        auto parameter_desc_expected = fe::graphics::ParameterDesc::from("g_ModelMatrices", shader_file_data.descriptor_layouts);
        if (!parameter_desc_expected.has_value()) {
            switch (parameter_desc_expected.error()) {
                case fe::graphics::ParameterDesc::ConversionErrors::FAILED_TO_FIND_DESCRIPTOR:
                    builder.assertError("Failed to find descriptor");
                    break;
                case fe::graphics::ParameterDesc::ConversionErrors::FAILED_TO_GET_SHADER_FILE_DATA_RESOURCE:
                    builder.assertError("Failed to find shader file data resource");
                    break;
                default:
                    builder.assertFatal("Unknown error");
                    return;
            }
        }
        auto parameter_expected = builder.renderer.CreateParameter(parameter_desc_expected.value());
        if (!parameter_expected.has_value()) {
            switch (parameter_expected.error()) {
                case fe::graphics::ParameterCreationErrors::FORGOT_TO_SPECIALIZE_GENERIC_DESCRIPTOR:
                    builder.assertError("Forgot to specialize a generic descriptor");
                    break;
                case fe::graphics::ParameterCreationErrors::MAPPED_MEMORY_WAS_NULLPTR:
                    builder.assertError("Mapped memory was nullptr");
                    break;
                case fe::graphics::ParameterCreationErrors::UNSUPPORTED_DESCRIPTOR_TYPE:
                    builder.assertError("Unsupported descriptor type");
                    break;
                default:
                    builder.assertFatal("Unknown error");
                    return;
            }
        }
        pass_data.model_matrices_parameter_id = parameter_expected.value();
        {
            auto it = std::ranges::find_if(shader_file_data.descriptor_layouts, [](const auto& e) -> bool {
                return e.name == "g_MaterialsRawData";
            });

            if (it == shader_file_data.descriptor_layouts.end()) {
                builder.assertFatal("Failed to find parameter for g_MaterialsRawData");
                return;
            }

            // TODO :
            //
            //builder.renderer.SpecializeParameter(*it, "OpenGLBuffer");

            fe::shader::ReflectedDescriptor specialized_parameter{ *it };
            specialized_parameter.descriptor_type = fe::shader::DescriptorType::STORAGE_BUFFER;
            specialized_parameter.array_size      = 0;
            specialized_parameter.size            = 0;

            auto model_matrices_parameter_expected = builder.renderer.CreateParameter(fe::graphics::ParameterDesc::from(specialized_parameter));
            if (!model_matrices_parameter_expected.has_value()) {
                builder.assertFatal("Failed to create parameter for g_MaterialsRawData");
                return;
            }
            pass_data.materials_parameter_id = model_matrices_parameter_expected.value();
        }
        {
            auto it = std::ranges::find_if(shader_file_data.descriptor_layouts, [](const auto& e) -> bool {
                return e.name == "g_GlobalData";
            });

            if (it == shader_file_data.descriptor_layouts.end()) {
                builder.assertFatal("Failed to find parameter for g_GlobalData");
                return;
            }

            auto model_matrices_parameter_expected = builder.renderer.CreateParameter(fe::graphics::ParameterDesc::from(*it));
            if (!model_matrices_parameter_expected.has_value()) {
                builder.assertFatal("Failed to create parameter for g_GlobalData");
                return;
            }
            pass_data.global_data_parameter_id = model_matrices_parameter_expected.value();
        }
        { // light projection
            auto parameter_desc_expected = fe::graphics::ParameterDesc::from("g_LightProjection", shader_file_data.descriptor_layouts);
            if (!parameter_desc_expected.has_value()) {
                switch (parameter_desc_expected.error()) {
                    case fe::graphics::ParameterDesc::ConversionErrors::FAILED_TO_FIND_DESCRIPTOR:
                        builder.assertError("Failed to find descriptor");
                        break;
                    case fe::graphics::ParameterDesc::ConversionErrors::FAILED_TO_GET_SHADER_FILE_DATA_RESOURCE:
                        builder.assertError("Failed to find shader file data resource");
                        break;
                    default:
                        builder.assertFatal("Unknown error");
                        return;
                }
            }
            auto parameter_expected = builder.renderer.CreateParameter(parameter_desc_expected.value());
            if (!parameter_expected.has_value()) {
                switch (parameter_expected.error()) {
                    case fe::graphics::ParameterCreationErrors::FORGOT_TO_SPECIALIZE_GENERIC_DESCRIPTOR:
                        builder.assertError("Forgot to specialize a generic descriptor");
                        break;
                    case fe::graphics::ParameterCreationErrors::MAPPED_MEMORY_WAS_NULLPTR:
                        builder.assertError("Mapped memory was nullptr");
                        break;
                    case fe::graphics::ParameterCreationErrors::UNSUPPORTED_DESCRIPTOR_TYPE:
                        builder.assertError("Unsupported descriptor type");
                        break;
                    default:
                        builder.assertFatal("Unknown error");
                        return;
                }
            }
            pass_data.light_projection_parameter = parameter_expected.value();
        }

        fe::resource::Material pbr_material{};
        pbr_material.layout_key                          = { .shader_file_data_ptr = shader_file_data_ptr, .structure_layout_storage_index = 0 };
        pbr_material.pipeline_flags_override.cull_enable = false;
        pbr_material.pipeline_flags_override.cull_mode   = fe::CullMode::NONE;

        fe::pointer<fe::resource::Material> pbr_material_ptr = builder.resource_manager.CreateResource(std::move(pbr_material));

        auto pipeline_result = fe::PBRPipelineBuilder::Build(shader_file_data_ptr,
                                                             pbr_material_ptr,
                                                             builder.resource_manager,
                                                             builder.renderer);
        if (pipeline_result.has_value()) {
            pass_data.pipeline_id = pipeline_result.value();
        }
        else {
            const auto& error             = pipeline_result.error();
            std::string error_code_string = std::to_string(static_cast<const uint8_t>(error.error_code));
            std::string error_string      = "Failed to create PBR effect material\nError code : " + error_code_string;

            if (error.detailed_message.has_value()) {
                error_string += "\nAdditional message :\n";

                const auto& detailed_message = error.detailed_message.value();
                if (std::holds_alternative<fe::graphics::PipelineCreationErrors>(detailed_message)) {
                    const auto& value = std::get<fe::graphics::PipelineCreationErrors>(detailed_message);
                    error_string += std::to_string(static_cast<const uint8_t>(value));
                }
                else if (std::holds_alternative<fe::graphics::ParameterCreationErrors>(detailed_message)) {
                    const auto& value = std::get<fe::graphics::ParameterCreationErrors>(detailed_message);
                    error_string += std::to_string(static_cast<const uint8_t>(value));
                }
                else if (std::holds_alternative<std::string>(detailed_message)) {
                    const auto& value = std::get<std::string>(detailed_message);
                    error_string += value;
                }
            }

            builder.assertFatal(error_string);
            return;
        }

        pass_data.default_material_ptr = pbr_material_ptr;

        pass_data.data.resize(256);
        pass_data.data[0] = glm::mat4{ 1.0f };
        pass_data.data[1] = glm::mat4{ 1.0f };
        pass_data.data[2] = glm::mat4{ 1.0f };

        std::vector<ForwardPassData::PBRMaterialData> all_materials(3);
        all_materials[0].base_color_texture_handle = 4294969866 + 2;
        all_materials[0].base_color_factor         = glm::vec4(1.0f);

        all_materials[1].base_color_texture_handle = 4294969865 + 2;
        all_materials[1].base_color_factor         = glm::vec4(1.0f);

        all_materials[2].base_color_texture_handle = 4294969867 + 2;
        all_materials[2].base_color_factor         = glm::vec4(1.0f);

        pass_data.materials_data.resize(sizeof(ForwardPassData::PBRMaterialData) * all_materials.size());
        memcpy(&pass_data.materials_data[0], all_materials.data(), sizeof(ForwardPassData::PBRMaterialData) * all_materials.size());

        //glm::mat4 proj = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.001f, 1000.0f);
        //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3{}, glm::vec3(0.0f, 1.0f, 0.0f));

        ForwardPassData::GlobalData global_data{};
        global_data.view = glm::lookAt(glm::vec3(8.0f, 3.0f, 5.0f),
                                       glm::vec3(-1.0f, -1.0f, 0.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f));

        global_data.projection = glm::perspective(glm::radians(45.0f),
                                                  1920.0f / 1080.0f,
                                                  0.1f,
                                                  1000.0f);

        pass_data.global_data_as_bytes.resize(sizeof(ForwardPassData::GlobalData));
        memcpy(&pass_data.global_data_as_bytes[0], &global_data, sizeof(ForwardPassData::GlobalData));
    }

    static void Execute(fe::RenderGraphContext& context, ForwardPassData& pass_data) {
        auto camera_view = context.render_registry.view<fe::component::ViewProjection>();
        for (auto entity : camera_view) {
            const auto& camera_component = context.render_registry.get<fe::component::ViewProjection>(entity);

            ForwardPassData::GlobalData global_data{};
            global_data.view       = camera_component.view;
            global_data.projection = camera_component.projection;

            pass_data.global_data_as_bytes.resize(sizeof(ForwardPassData::GlobalData));
            memcpy(&pass_data.global_data_as_bytes[0], &global_data, sizeof(ForwardPassData::GlobalData));
        }

        auto transform_view = context.render_registry.view<fe::component::LocalToWorld>();
        transform_view.each([&pass_data, i = 0](const fe::component::LocalToWorld& local_to_world_component) mutable {
            pass_data.data[i] = local_to_world_component.matrix;
            i                 = (i + 1) % 2;
        });

        context.BindPipeline(pass_data.pipeline_id);

        context.BindBuffer(pass_data.model_matrices_parameter_id);
        context.BindBuffer(pass_data.global_data_parameter_id);
        context.BindBuffer(pass_data.materials_parameter_id);
        context.BindBuffer(pass_data.light_projection_parameter);

        context.WriteBuffer(pass_data.model_matrices_parameter_id, pass_data.data);
        context.WriteBuffer(pass_data.materials_parameter_id, pass_data.materials_data);
        context.WriteBuffer(pass_data.global_data_parameter_id, pass_data.global_data_as_bytes);

        //auto        light_entity    = context.render_registry.view<fe::component::Light>().back();
        //const auto& light_component = context.render_registry.get<fe::component::Light>(light_entity);

        //context.WriteBuffer(pass_data.light_projection_parameter, std::as_bytes(std::span{ &light_component.transform, 1 }));

        auto models_view = context.render_registry.view<fe::component::Model>();
        models_view.each([&context, i = 0](const fe::component::Model& model_component) mutable {
            context.DrawModel(model_component.model_ptr, i);
            i = (i + 1) % 2;
        });
    }

    ForwardPass()  = default;
    ~ForwardPass() = default;
};

void on_initialize(fe::context& context, fe::dll_api::module_context& self) {
    //static std::unique_ptr<entt::registry> local_registry = std::make_unique<entt::registry>();
    //context.emplace(local_registry, fe::make_hash("game"));

    auto& registry = context.get<entt::registry>();

    registry.emplace<game::components::MyComponent>(registry.create());

    auto& renderer     = context.get<fe::IRenderer>();
    auto& render_graph = context.get<fe::RenderGraph>();

    auto forward_pass_handle = render_graph.AddPass<ForwardPass>(fe::fixed_string_view<32>{ "ForwardPass" });
    self.own_scoped(fe::ScopedRenderPassHandle<ForwardPass>{ &render_graph, forward_pass_handle });

    auto create_command_list = render_graph.Compile();
    auto mapping_result      = renderer.CreateRenderGraphResources(create_command_list);
    render_graph.SetupResourceBindings(mapping_result);

    //auto& resource_manager = context.get<fe::ResourceManager>();

    //Data data{};
    //data.model_handle = resource_manager.ImportResource<fe::resource::Model>(fe::PATH.getModelsPath() / u8"FlightHelmet" / u8"FlightHelmet.gltf");

    //context.emplace<Data>(data);
}

static size_t working_time{};

void on_update(fe::context& context, fe::dll_api::module_context& self) {
    auto& platform_system       = context.get<fe::IPlatformSystem>();
    auto& primary_window_handle = context.get<fe::WindowHandle>();
    auto& main_event_bus        = context.get<fe::EventBus>();
    auto& renderer              = context.get<fe::IRenderer>();
    auto& render_graph          = context.get<fe::RenderGraph>();
    auto& registry              = context.get<entt::registry>();

    auto& primary_window = *platform_system.getWindow(primary_window_handle);

    primary_window.PollEvents();

    main_event_bus.DispatchAll();
    main_event_bus.Clear();

    renderer.BeginFrame();

    fe::RenderGraphCollector<fe::component::Transform,
                             fe::component::Camera,
                             fe::component::ViewProjection,
                             fe::component::LocalToWorld,
                             fe::component::Model,
                             fe::component::Light>
        collector{ registry };

    auto render_command_list = render_graph.Execute(collector.getRegistry());

    renderer.EndFrame(render_command_list);

    working_time++;
}

void on_release(fe::context& context, fe::dll_api::module_context& self) {
    auto& registry = context.get<entt::registry>();

    for (auto [id, storage] : registry.storage()) {
#define GAME_GENERATE_COMPONENTS_TYPE_IDS(COMPONENT_NAME)                 \
    if (entt::type_id<game::components::COMPONENT_NAME>().hash() == id) { \
        registry.reset(id);                                               \
    }

        GAME_COMPONENTS_LIST(GAME_GENERATE_COMPONENTS_TYPE_IDS)

#undef GAME_GENERATE_COMPONENTS_TYPE_IDS
    }

    fe::logging::info("dll unloaded\nit worked for %llu frames", working_time);
}

FORR_REGISTER_DLL_MODULE("ForrGame-Example", true, &on_initialize, &on_update, &on_release)
