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

    struct ForwardPassData {
        fe::pointer<resource::ShaderProgram> default_shader_program_ptr{};
        fe::pointer<resource::Material>      default_material_ptr{};
        fe::pointer<resource::Model>         test_model_ptr{};
        ParameterID                          model_matrices_parameter_id{};
        std::vector<glm::mat4> data{};
    };
    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder, ForwardPassData& pass_data) { // setup can be called twice
            builder.readImage(fe::string_hash("ShadowMap"), ResourceState::SHADER_READ_ONLY);
            builder.writeToScreen(true);

            if (!pass_data.default_shader_program_ptr) {
                fe::pointer<resource::ShaderFileData> shader_file_data_ptr = builder.resource_manager.ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Default\\PBRMaterial\\shader.slang");
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
                        return descriptor.name == fe::hashed_string{ "model_matrices" };
                    });

                    if (it == descriptors.reflected_layout.descriptors.end()) {
                        fe::logging::error("Failed to find model_matrices");
                    }
                    else {
                        const auto& descriptor                = *it;
                        pass_data.model_matrices_parameter_id = builder.renderer.CreateParameter(descriptor);
                    }
                }
            }
            pass_data.default_material_ptr = builder.resource_manager.GetContext().default_gltf_material_ptr;
            if (!pass_data.test_model_ptr) {
                pass_data.test_model_ptr = builder.resource_manager.ImportResource<resource::Model>(PATH.getModelsPath() / "TatarSuzanne\\TatarSuzanne.gltf");
            }

            pass_data.data.resize(256);
            pass_data.data[0] = glm::mat4{1, 0, 0, 0,
                                          0, 1, 0, 0,
                                          0, 0, 1, 0,
                                          0, 0, 0, 1};
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
            context.BindBuffer(pass_data.model_matrices_parameter_id);
            context.WriteBuffer(pass_data.model_matrices_parameter_id, pass_data.data);

            context.BindShaderProgram(pass_data.default_shader_program_ptr);
            context.BindMaterial(pass_data.default_material_ptr);

            context.BindModel(pass_data.test_model_ptr); // temp
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

} // namespace fe
