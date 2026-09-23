/*===============================================

    Forr Engine

    File : DefaultPipelineBuilders.hpp
    Role : pipeline builders that provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IRenderer.hpp"

namespace fe {
    // TODO : move parameter ids to some new 'render context', they mustn't be here
    //
    // 'effects' are structures that contain everything that was built by the builder
    struct FORR_API PBREffectMaterial {
        fe::PipelineID pipeline_id{};

        fe::ParameterID materials_raw_data_parameter_id{};
        fe::ParameterID global_data_parameter_id{};
        fe::ParameterID model_matrices_parameter_id{};

        // helper-function to not write this lines of code multiple times
        // binds everything in the structure : pipeline and parameters
        void bind(IRenderer& renderer) {
            renderer.BindPipeline(pipeline_id);
            renderer.BindParameter(materials_raw_data_parameter_id);
            renderer.BindParameter(global_data_parameter_id);
            renderer.BindParameter(model_matrices_parameter_id);
        }

        // helper function to not write this lines of code multiple times
        //  make sure you don't call this in the de-constructor
        // destroys everything in the structure : pipeline and parameters
        void free(IRenderer& renderer) {
            renderer.DestroyParameter(materials_raw_data_parameter_id);
            renderer.DestroyParameter(global_data_parameter_id);
            renderer.DestroyParameter(model_matrices_parameter_id);
            renderer.DestroyPipeline(pipeline_id);
        }
    };

    enum class PBREffectErrorCodes : uint8_t {
        SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_PTR_WAS_INVALID,
        MATERIAL_LAYOUT_SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_LAYOUT_STRUCTURE_INDEX_WAS_INVALID,
        VERTEX_ENTRY_POINT_ABSENT,
        FRAGMENT_ENTRY_POINT_ABSENT,
        UNKNOWN_GRAPHICS_BACKEND,
        FAILED_TO_CREATE_PIPELINE,
        FAILED_TO_CREATE_PARAMETER,
        DESCRIPTOR_ABSENT
    };

    struct FORR_API PBREffectError {
        using DetailedMessageVariants = std::variant<PipelineCreationErrors,
                                                     ParameterCreationErrors,
                                                     fe::hashed_string>;

        PBREffectErrorCodes                    error_code{};
        std::optional<DetailedMessageVariants> detailed_message{}; // used only in specific cases

        PBREffectError(PBREffectErrorCodes error_code)
            : error_code(error_code) {}
        PBREffectError(PBREffectErrorCodes error_code, DetailedMessageVariants detailed_message)
            : error_code(error_code), detailed_message(std::move(detailed_message)) {}
    };

    class FORR_API PBREffectBuilder {
    public:
        static std::expected<PBREffectMaterial, PBREffectError> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                      fe::pointer<resource::Material>       material_ptr,
                                                                      ResourceManager&                      resource_manager,
                                                                      IRenderer&                            renderer) {
            /* declare common shader resources */
            inline const static fe::hashed_string              vertex_entry_point_name{ "vertexMain" };
            inline const static fe::hashed_string              fragment_entry_point_name{ "fragmentMain" };
            inline const static fe::hashed_string              material_interface_name{ "IMaterial" };
            inline const static fe::hashed_string              materials_raw_data_name{ "g_MaterialsRawData" };
            inline const static fe::hashed_string              model_matrices_name{ "g_ModelMatrices" };
            inline const static fe::hashed_string              global_data_name{ "g_GlobalData" };
            inline const static std::vector<fe::hashed_string> descriptor_set_names{ materials_raw_data_name,
                                                                                     model_matrices_name,
                                                                                     global_data_name };
            inline const static fe::hashed_string              push_constants_name{ "push_constants" };
            inline const static fe::hashed_string              unspecialized_buffer_name{ "TBuffer" };
            inline const static fe::hashed_string              opengl_buffer_name{ "OpenGLBuffer" };
            inline const static fe::hashed_string              vulkan_buffer_name{ "VulkanBuffer" };

            // load shader file data and check for error
            auto shader_file_data_optional = resource_manager.GetResource(shader_file_data_ptr);
            if (!shader_file_data_optional.has_value())
                return std::unexpected{ PBREffectErrorCodes::SHADER_FILE_DATA_PTR_WAS_INVALID };

            // load material and check for error
            auto material_optional = resource_manager.GetResource(material_ptr);
            if (!material_optional.has_value())
                return std::unexpected{ PBREffectErrorCodes::MATERIAL_PTR_WAS_INVALID };

            /* translate 'optionals' to 'T&' */
            auto& shader_file_data = shader_file_data_optional.value();
            auto& material         = material_optional.value();

            // pre-create pipeline desc
            PipelineDesc pipeline_desc{
                .pipeline_flags   = material.pipeline_flags_override,
                .shader_file_ptrs = { shader_file_data_ptr },
                .entry_points     = { vertex_entry_point_name, fragment_entry_point_name },
                .descriptor_sets  = descriptor_set_names,
                .push_constants   = { push_constants_name },
            };

            // this is needed to write directly into 'pipeline_desc' without extra copyings
            auto& specialization = pipeline_desc.specialization.emplace();

            // shader file data where we gonna search for material's structure
            std::reference_wrapper<resource::ShaderFileData> shader_file_data_to_find_material_structure{ shader_file_data };

            // load material's shader file data if they are not from the same file
            if (material.layout_key.shader_file_data != shader_file_data_ptr) {
                auto material_shader_file_data_optional = resource_manager.GetResource(material.layout_key.shader_file_data);
                if (!material_shader_file_data_optional.has_value())
                    return std::unexpected{ PBREffectErrorCodes::MATERIAL_LAYOUT_SHADER_FILE_DATA_PTR_WAS_INVALID };

                pipeline_desc.shader_file_ptrs.emplace_back(material.layout_key.shader_file_data);
                shader_file_data_to_find_material_structure = material_shader_file_data_optional.value();
            }

            size_t layout_index      = material.layout_key.structure_layout_storage_index;
            auto&  structure_layouts = shader_file_data_to_find_material_structure.get().structure_layouts;

            if (layout_index >= structure_layouts.size()) {
                return std::unexpected{ PBREffectErrorCodes::MATERIAL_LAYOUT_STRUCTURE_INDEX_WAS_INVALID };
            }

            const auto& material_structure_layout = structure_layouts[layout_index]; // found material's structure layout

            // find what generic arguments every entry point need and fill up the 'specialization'
            specialization.entry_points.reserve(pipeline_desc.entry_points.size());
            for (const auto& entry_point : pipeline_desc.entry_points) {
                auto it = std::ranges::find_if(shader_file_data.entry_points, [&entry_point](const auto& e) -> bool {
                    return e.name == entry_point;
                });

                if (it == shader_file_data.entry_points.end()) {
                    return std::unexpected{ entry_point == vertex_entry_point_name ? PBREffectErrorCodes::VERTEX_ENTRY_POINT_ABSENT
                                                                                   : PBREffectErrorCodes::FRAGMENT_ENTRY_POINT_ABSENT };
                }

                specialization.entry_points.emplace_back(shader::EntryPointSpecialization{ .name      = entry_point,
                                                                                           .arguments = { shader::SpecializationArgument{ .name  = material_interface_name,
                                                                                                                                          .value = material_structure_layout.name } } });
            }

            std::reference_wrapper<const fe::hashed_string> buffer_specialization_name{ opengl_buffer_name };

            switch (renderer.GetCurrentGraphicsBackend()) {
                case GraphicsBackend::OpenGL:
                    // nothing here, it's already set to OpenGL
                    break;
                case GraphicsBackend::Vulkan:
                    buffer_specialization_name = vulkan_buffer_name;
                    break;
                default:
                    return std::unexpected{ PBREffectErrorCodes::UNKNOWN_GRAPHICS_BACKEND };
            }

            // this is necessary to specialize at least 'g_MaterialsRawData'
            specialization.global_arguments.emplace_back(shader::SpecializationArgument{ .name  = unspecialized_buffer_name,
                                                                                         .value = buffer_specialization_name.get() });

            PBREffectMaterial pbr_effect_material{};

            // create pipeline
            auto pipeline_id_expected = renderer.CreatePipeline(pipeline_desc);
            if (pipeline_id_expected.has_value()) {
                pbr_effect_material.pipeline_id = pipeline_id_expected.value();
            }
            else {
                return std::unexpected{ PBREffectError{ PBREffectErrorCodes::FAILED_TO_CREATE_PIPELINE, pipeline_id_expected.error() } };
            }

            // create all parameters
            for (const auto& descriptor_set : pipeline_desc.descriptor_sets) {
                auto it = std::ranges::find_if(shader_file_data.descriptor_layouts, [&descriptor_set](const auto& e) -> bool {
                    return e.name == descriptor_set;
                });

                if (it == shader_file_data.descriptor_layouts.end()) {
                    pbr_effect_material.free(renderer);
                    return std::unexpected{ PBREffectError{ PBREffectErrorCodes::DESCRIPTOR_ABSENT, descriptor_set } };
                }

                auto parameter_expected = renderer.CreateParameter(*it);
                if (parameter_expected.has_value()) {
                    if (it->name == materials_raw_data_name) { // g_MaterialsRawData
                        pbr_effect_material.materials_raw_data_parameter_id = parameter_expected.value();
                    }
                    else if (it->name == model_matrices_name) { // g_ModelMatrices
                        pbr_effect_material.model_matrices_parameter_id = parameter_expected.value();
                    }
                    else if (it->name == global_data_name) { // g_GlobalData
                        pbr_effect_material.global_data_parameter_id = parameter_expected.value();
                    }
                }
                else {
                    pbr_effect_material.free(renderer);
                    return std::unexpected{ PBREffectError{ PBREffectErrorCodes::FAILED_TO_CREATE_PARAMETER, parameter_expected.error() } };
                }
            }

            return pbr_effect_material;
        }
    };
} // namespace fe
