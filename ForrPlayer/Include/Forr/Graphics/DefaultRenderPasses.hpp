/*===============================================

    Forr Engine

    File : DefaultRenderPasses.hpp
    Role : render passes that provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "RenderGraph.hpp"
#include "ECS/Components.hpp"

namespace fe {
    struct ShadowPassData {
        fe::pointer<resource::ShaderProgram> shadow_shader_program_ptr{};
    };
    struct ShadowPass {
        static void Setup(RenderGraphBuilder& builder, ShadowPassData& pass_data) {
            builder.createImage(render_graph::ImageDesc{ fe::string_hash("ShadowMap"),
                                                         render_graph::ImageType::IMAGE_TYPE_2D,
                                                         render_graph::Format::RGBA8_SRGB,
                                                         glm::ivec3{ 3840, 2160, 1 },
                                                         1,
                                                         render_graph::ImageUsageBits::RENDER_TARGET });
            builder.writeImage(fe::string_hash("ShadowMap"), ResourceState::RENDER_TARGET);

            if (!pass_data.shadow_shader_program_ptr) {
                // it should work like this, I guess
                fe::pointer<resource::ShaderFileData> shader_file_data_ptr = builder.resource_manager.ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Shadow.slang");

                const resource::ShaderFileData& shader_file_data = *builder.resource_manager.GetResource(shader_file_data_ptr);
                if (!shader_file_data.shader_program_ptr.has_value()) {
                    builder.assertFatal("No shadow shader");
                    return;
                }

                pass_data.shadow_shader_program_ptr = shader_file_data.shader_program_ptr.value();
            }
        }

        static void Execute(RenderGraphContext& context, ShadowPassData& pass_data) {
            auto view = context.render_registry.view<TransformComponent>();
            // ...
        }

        ShadowPass()  = default;
        ~ShadowPass() = default;
    };

    struct ForwardPassData { // everything is temp
        fe::pointer<resource::ShaderProgram> default_shader_program_ptr{};

        fe::pointer<resource::Material> default_material_ptr{};
        fe::pointer<resource::Model>    test_model_ptr{};
        fe::pointer<resource::Model>    test_model2_ptr{};

        ParameterID            model_matrices_parameter_id{};
        std::vector<glm::mat4> data{};

        struct alignas(16) PBRMaterialData {
            uint64_t  base_color_texture_handle{};
            glm::vec4 base_color_factor{};
        };
        ParameterID            materials_parameter_id{};
        std::vector<std::byte> materials_data{};

        struct alignas(16) GlobalData {
            glm::mat4 view{};
            glm::mat4 projection{};

            GlobalData() = default;
        };
        ParameterID            global_data_parameter_id{};
        std::vector<std::byte> global_data_as_bytes{};

        struct alignas(16) PushConstants {
            int instance_index;
            int material_buffer_offset;

            PushConstants() = default;
        };
        std::vector<std::byte> push_constants_data{};
        std::vector<std::byte> push_constants_data2{};

        float time{};
    };
    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder, ForwardPassData& pass_data) { // setup can be called twice
            builder.writeToScreen(true);

            if (!pass_data.default_shader_program_ptr) {
                fe::pointer<resource::ShaderFileData> shader_file_data_ptr = builder.resource_manager.ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Default\\PBRMaterial\\PBRMaterial.slang");
                const resource::ShaderFileData&       shader_file_data     = *builder.resource_manager.GetResource(shader_file_data_ptr);
                if (!shader_file_data.shader_program_ptr.has_value()) {
                    builder.assertFatal("No shader");
                    return;
                }
                pass_data.default_shader_program_ptr = shader_file_data.shader_program_ptr.value();

                if (pass_data.model_matrices_parameter_id.storage_index == ~0) {
                    const auto& shader_program = *builder.resource_manager.GetResource(pass_data.default_shader_program_ptr);
                    const auto& descriptors    = *builder.resource_manager.GetResource(shader_program.descriptors_layout_ptr.value());

                    auto it = std::ranges::find_if(descriptors.reflected_layout.descriptors, [](const shader::ReflectedDescriptor& descriptor) -> bool {
                        return descriptor.name == fe::hashed_string{ "g_ModelMatrices" };
                    });

                    if (it == descriptors.reflected_layout.descriptors.end()) {
                        fe::logging::error("Failed to find g_ModelMatrices");
                    }
                    else {
                        const auto& descriptor                = *it;
                        pass_data.model_matrices_parameter_id = builder.renderer.CreateParameter(descriptor);
                    }
                }

                if (pass_data.materials_parameter_id.storage_index == ~0) {
                    const auto& shader_program = *builder.resource_manager.GetResource(pass_data.default_shader_program_ptr);
                    const auto& descriptors    = *builder.resource_manager.GetResource(shader_program.descriptors_layout_ptr.value());

                    auto it = std::ranges::find_if(descriptors.reflected_layout.descriptors, [](const shader::ReflectedDescriptor& descriptor) -> bool {
                        return descriptor.name == fe::hashed_string{ "g_MaterialsRawData" };
                    });

                    if (it == descriptors.reflected_layout.descriptors.end()) {
                        fe::logging::error("Failed to find g_MaterialsRawData");
                    }
                    else {
                        const auto& descriptor           = *it;
                        pass_data.materials_parameter_id = builder.renderer.CreateParameter(descriptor);
                    }
                }

                if (pass_data.global_data_parameter_id.storage_index == ~0) {
                    const auto& shader_program = *builder.resource_manager.GetResource(pass_data.default_shader_program_ptr);
                    const auto& descriptors    = *builder.resource_manager.GetResource(shader_program.descriptors_layout_ptr.value());

                    auto it = std::ranges::find_if(descriptors.reflected_layout.descriptors, [](const shader::ReflectedDescriptor& descriptor) -> bool {
                        return descriptor.name == fe::hashed_string{ "g_GlobalData" };
                    });

                    if (it == descriptors.reflected_layout.descriptors.end()) {
                        fe::logging::error("Failed to find g_GlobalData");
                    }
                    else {
                        const auto& descriptor             = *it;
                        pass_data.global_data_parameter_id = builder.renderer.CreateParameter(descriptor);
                    }
                }
            }
            pass_data.default_material_ptr = builder.resource_manager.GetContext().default_pbr_material_ptr;
            if (!pass_data.test_model_ptr) {
                pass_data.test_model_ptr = builder.resource_manager.ImportResource<resource::Model>(PATH.getModelsPath() / "TatarSuzanne\\TatarSuzanne.gltf");
            }
            if (!pass_data.test_model2_ptr) {
                pass_data.test_model2_ptr = builder.resource_manager.ImportResource<resource::Model>(PATH.getModelsPath() / "FlightHelmet\\FlightHelmet.gltf");
            }

            pass_data.data.resize(256);
            pass_data.data[0] = glm::mat4{ 1.0f };
            pass_data.data[1] = glm::mat4{ 1.0f };

            std::vector<ForwardPassData::PBRMaterialData> all_materials(2);
            all_materials[0].base_color_texture_handle = 4294969856;
            all_materials[0].base_color_factor         = glm::vec4(1.0f);

            all_materials[1].base_color_texture_handle = 4294969859;
            all_materials[1].base_color_factor         = glm::vec4(1.0f);

            pass_data.materials_data.resize(sizeof(ForwardPassData::PBRMaterialData) * all_materials.size());
            memcpy(&pass_data.materials_data[0], all_materials.data(), sizeof(ForwardPassData::PBRMaterialData) * all_materials.size());

            ForwardPassData::GlobalData global_data{};
            global_data.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 6.0f),
                                           glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f));

            global_data.projection = glm::perspective(glm::radians(45.0f),
                                                      1920.0f / 1080.0f,
                                                      0.1f,
                                                      1000.0f);

            pass_data.global_data_as_bytes.resize(sizeof(ForwardPassData::GlobalData));
            memcpy(&pass_data.global_data_as_bytes[0], &global_data, sizeof(ForwardPassData::GlobalData));

            pass_data.push_constants_data.resize(sizeof(ForwardPassData::PushConstants));
            pass_data.push_constants_data2.resize(sizeof(ForwardPassData::PushConstants));
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
            model           = glm::rotate(model, pass_data.time, glm::vec3(0, 1, 0));
            model           = glm::scale(model, glm::vec3(0.7f));

            pass_data.data[0] = model;

            glm::mat4 model2 = glm::mat4(1.0f);
            model2           = glm::translate(model2, glm::vec3(1.0f, 0.0f, 0.0f));
            model2           = glm::rotate(model2, pass_data.time, glm::vec3(0, 1, 0));
            model2           = glm::scale(model2, glm::vec3(3.0f));

            pass_data.data[1] = model2;

            context.BindBuffer(pass_data.model_matrices_parameter_id);
            context.WriteBuffer(pass_data.model_matrices_parameter_id, pass_data.data);

            context.BindBuffer(pass_data.materials_parameter_id);
            context.WriteBuffer(pass_data.materials_parameter_id, pass_data.materials_data);

            context.BindBuffer(pass_data.global_data_parameter_id);
            context.WriteBuffer(pass_data.global_data_parameter_id, pass_data.global_data_as_bytes);

            context.BindShaderProgram(pass_data.default_shader_program_ptr);
            context.BindMaterial(pass_data.default_material_ptr);

            ForwardPassData::PushConstants push_constants{};
            push_constants.instance_index         = 0;
            push_constants.material_buffer_offset = 0;

            memcpy(pass_data.push_constants_data.data(), &push_constants, sizeof(ForwardPassData::PushConstants));
            context.BindModel(pass_data.test_model_ptr, pass_data.push_constants_data); // means 'draw'

            ForwardPassData::PushConstants push_constants2{};
            push_constants2.instance_index         = 1;
            push_constants2.material_buffer_offset = sizeof(ForwardPassData::PBRMaterialData);

            memcpy(pass_data.push_constants_data2.data(), &push_constants2, sizeof(ForwardPassData::PushConstants));
            context.BindModel(pass_data.test_model2_ptr, pass_data.push_constants_data2); // means 'draw'

            pass_data.time += 0.01f;
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

} // namespace fe
