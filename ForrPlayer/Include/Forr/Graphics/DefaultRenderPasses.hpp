/*===============================================

    Forr Engine

    File : DefaultRenderPasses.hpp
    Role : render passes that provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ECS/Components.hpp"
#include "DefaultPipelineBuilders.hpp"

namespace fe {
    struct ForwardPassData { // everything is temp
        PipelineID pipeline_id{};

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

        float time{};
    };
    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder, ForwardPassData& pass_data) { // setup can be called twice
            builder.writeToScreen(true);

            fe::pointer<resource::ShaderFileData> shader_file_data_ptr = builder.resource_manager.ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Default\\PBRMaterial\\PBRMaterial.slang");

            auto pipeline_result = PBRPipelineBuilder::Build(shader_file_data_ptr,
                                                           builder.resource_manager.GetContext().default_pbr_material_ptr,
                                                           builder.resource_manager,
                                                           builder.renderer);
            if (pipeline_result.has_value()) {
                pass_data.pipeline_id = pipeline_result.value();
            }
            else {
                const auto& error = pipeline_result.error();
                std::string error_code_string = std::to_string(static_cast<const uint8_t>(error.error_code));
                std::string error_string      = "Failed to create PBR effect material via default PBR material ptr from resource manager\nError code : " + error_code_string;
                
                if (error.detailed_message.has_value()) {
                    error_string += "\nAdditional message : ";
                    
                    const auto& detailed_message = error.detailed_message.value();
                    if (std::holds_alternative<PipelineCreationErrors>(detailed_message)) {
                        const auto& value = std::get<PipelineCreationErrors>(detailed_message);
                        error_string += std::to_string(static_cast<const uint8_t>(value));
                    }
                    else if (std::holds_alternative<ParameterCreationErrors>(detailed_message)) {
                        const auto& value = std::get<ParameterCreationErrors>(detailed_message);
                        error_string += std::to_string(static_cast<const uint8_t>(value));
                    }
                    else if (std::holds_alternative<fe::hashed_string>(detailed_message)) {
                        const auto& value = std::get<fe::hashed_string>(detailed_message);
                        error_string += value;
                    }
                }

                builder.assertFatal(error_string);
                return;
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

            all_materials[1].base_color_texture_handle = 4294969857;
            all_materials[1].base_color_factor         = glm::vec4(1.0f);

            pass_data.materials_data.resize(sizeof(ForwardPassData::PBRMaterialData) * all_materials.size());
            memcpy(&pass_data.materials_data[0], all_materials.data(), sizeof(ForwardPassData::PBRMaterialData) * all_materials.size());

            ForwardPassData::GlobalData global_data{};
            global_data.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
                                           glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 1.0f, 0.0f));

            global_data.projection = glm::perspective(glm::radians(45.0f),
                                                      1920.0f / 1080.0f,
                                                      0.1f,
                                                      1000.0f);

            pass_data.global_data_as_bytes.resize(sizeof(ForwardPassData::GlobalData));
            memcpy(&pass_data.global_data_as_bytes[0], &global_data, sizeof(ForwardPassData::GlobalData));
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
            model           = glm::rotate(model, pass_data.time, glm::vec3(0, 1, 0));
            model           = glm::scale(model, glm::vec3(0.7f));

            pass_data.data[0] = model;

            glm::mat4 model2 = glm::mat4(1.0f);
            model2           = glm::translate(model2, glm::vec3(1.5f, -0.5f, 0.0f));
            model2           = glm::rotate(model2, pass_data.time, glm::vec3(0, 1, 0));
            model2           = glm::scale(model2, glm::vec3(2.0f));

            pass_data.data[1] = model2;

            context.BindBuffer(pass_data.model_matrices_parameter_id);
            context.WriteBuffer(pass_data.model_matrices_parameter_id, pass_data.data);

            context.BindBuffer(pass_data.materials_parameter_id);
            context.WriteBuffer(pass_data.materials_parameter_id, pass_data.materials_data);

            context.BindBuffer(pass_data.global_data_parameter_id);
            context.WriteBuffer(pass_data.global_data_parameter_id, pass_data.global_data_as_bytes);

            context.DrawModel(pass_data.test_model_ptr, 0);
            context.DrawModel(pass_data.test_model2_ptr, 1);

            pass_data.time += 0.01f;
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

} // namespace fe
