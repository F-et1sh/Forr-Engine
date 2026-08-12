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
    };
    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder, ForwardPassData& pass_data) { // setup can be called twice
            builder.writeToScreen(true);

            //builder.importContext("context.slang"); // TODO : provide this and handle conflicts

            if (!pass_data.default_shader_program_ptr) {
                fe::pointer<resource::ShaderFileData> shader_file_data_ptr = builder.resource_manager.ImportResource<resource::ShaderFileData>(PATH.getShadersPath() / "Default\\PBRMaterial\\shader.slang");
                const resource::ShaderFileData&       shader_file_data     = *builder.resource_manager.GetResource(shader_file_data_ptr);
                if (!shader_file_data.shader_program_ptr.has_value()) {
                    builder.assertFatal("No shader");
                    return;
                }
                pass_data.default_shader_program_ptr = shader_file_data.shader_program_ptr.value();
            }
            pass_data.default_material_ptr = builder.resource_manager.GetContext().default_gltf_material_ptr;
            if (!pass_data.test_model_ptr) {
                pass_data.test_model_ptr = builder.resource_manager.ImportResource<resource::Model>(PATH.getModelsPath() / "TatarSuzanne\\TatarSuzanne.gltf");
            }
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
            //context.BindDescriptors(PATH.getShadersPath() / "Default\\PBRMaterial\\shader.slang"); // TODO : provide this and handle conflicts

            auto view = context.render_registry.view<TransformComponent, MeshComponent>();

            context.BindShaderProgram(pass_data.default_shader_program_ptr);

            context.BindMaterial(pass_data.default_material_ptr);

            // temp
            context.BindModel(pass_data.test_model_ptr);

            //for (const auto& [entity, transform_component, mesh_component] : view.each()) {
            //    context.BindShaderProgram(pass_data.default_shader_program_ptr);
            //    context.BindMaterial(pass_data.default_material_ptr);
            //
            //    //context.BindModel(mesh_component.model_ptr); // TODO : rewrite this

            //    //context.DrawIndexed(render_graph::DrawIndexed{ 3, 1, 0, 0, 0 });
            //}
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

} // namespace fe
