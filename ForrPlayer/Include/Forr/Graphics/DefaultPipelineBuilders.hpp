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
    enum class PBRPipelineErrorCodes : uint8_t {
        SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_PTR_WAS_INVALID,
        MATERIAL_LAYOUT_SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_LAYOUT_STRUCTURE_INDEX_WAS_INVALID,
        VERTEX_ENTRY_POINT_ABSENT,
        FRAGMENT_ENTRY_POINT_ABSENT,
        UNKNOWN_GRAPHICS_BACKEND,
        FAILED_TO_CREATE_PIPELINE,
    };

    struct FORR_API PBRPipelineError {
        using DetailedMessageVariants = std::variant<PipelineCreationErrors,
                                                     ParameterCreationErrors,
                                                     fe::hashed_string>;

        PBRPipelineErrorCodes                  error_code{};
        std::optional<DetailedMessageVariants> detailed_message{}; // used only in specific cases

        PBRPipelineError(PBRPipelineErrorCodes error_code)
            : error_code(error_code) {}
        PBRPipelineError(PBRPipelineErrorCodes error_code, DetailedMessageVariants detailed_message)
            : error_code(error_code), detailed_message(std::move(detailed_message)) {}
    };

    class FORR_API PBRPipelineBuilder {
    public:
        static std::expected<fe::PipelineID, PBRPipelineError> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
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
                return std::unexpected{ PBRPipelineErrorCodes::SHADER_FILE_DATA_PTR_WAS_INVALID };

            // load material and check for error
            auto material_optional = resource_manager.GetResource(material_ptr);
            if (!material_optional.has_value())
                return std::unexpected{ PBRPipelineErrorCodes::MATERIAL_PTR_WAS_INVALID };

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
                    return std::unexpected{ PBRPipelineErrorCodes::MATERIAL_LAYOUT_SHADER_FILE_DATA_PTR_WAS_INVALID };

                pipeline_desc.shader_file_ptrs.emplace_back(material.layout_key.shader_file_data);
                shader_file_data_to_find_material_structure = material_shader_file_data_optional.value();
            }

            size_t layout_index      = material.layout_key.structure_layout_storage_index;
            auto&  structure_layouts = shader_file_data_to_find_material_structure.get().structure_layouts;

            if (layout_index >= structure_layouts.size()) {
                return std::unexpected{ PBRPipelineErrorCodes::MATERIAL_LAYOUT_STRUCTURE_INDEX_WAS_INVALID };
            }

            const auto& material_structure_layout = structure_layouts[layout_index]; // found material's structure layout

            // find what generic arguments every entry point need and fill up the 'specialization'
            specialization.entry_points.reserve(pipeline_desc.entry_points.size());
            for (const auto& entry_point : pipeline_desc.entry_points) {
                auto it = std::ranges::find_if(shader_file_data.entry_points, [&entry_point](const auto& e) -> bool {
                    return e.name == entry_point;
                });

                if (it == shader_file_data.entry_points.end()) {
                    return std::unexpected{ entry_point == vertex_entry_point_name ? PBRPipelineErrorCodes::VERTEX_ENTRY_POINT_ABSENT
                                                                                   : PBRPipelineErrorCodes::FRAGMENT_ENTRY_POINT_ABSENT };
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
                    return std::unexpected{ PBRPipelineErrorCodes::UNKNOWN_GRAPHICS_BACKEND };
            }

            // this is necessary to specialize at least 'g_MaterialsRawData'
            specialization.global_arguments.emplace_back(shader::SpecializationArgument{ .name  = unspecialized_buffer_name,
                                                                                         .value = buffer_specialization_name.get() });

            // create pipeline
            auto pipeline_id_expected = renderer.CreatePipeline(pipeline_desc);
            if (!pipeline_id_expected.has_value()) {
                return std::unexpected{ PBRPipelineError{ PBRPipelineErrorCodes::FAILED_TO_CREATE_PIPELINE, pipeline_id_expected.error() } };
            }

            return pipeline_id_expected.value();
        }
    };

    enum class SolidColorPipelineErrorCodes {
        SHADER_FILE_DATA_PTR_WAS_INVALID,
        VERTEX_ENTRY_POINT_ABSENT,
        FRAGMENT_ENTRY_POINT_ABSENT,
        UNKNOWN_GRAPHICS_BACKEND,
        FAILED_TO_CREATE_PIPELINE,
    };

    struct FORR_API SolidColorPipelineError {
        using DetailedMessageVariants = std::variant<SolidColorPipelineErrorCodes,
                                                     ParameterCreationErrors,
                                                     fe::hashed_string>;

        SolidColorPipelineErrorCodes           error_code{};
        std::optional<DetailedMessageVariants> detailed_message{}; // used only in specific cases

        SolidColorPipelineError(SolidColorPipelineErrorCodes error_code)
            : error_code(error_code) {}
        SolidColorPipelineError(SolidColorPipelineErrorCodes error_code, DetailedMessageVariants detailed_message)
            : error_code(error_code), detailed_message(std::move(detailed_message)) {}
    };

    class FORR_API SolidColorPipelineBuilder {
        static std::expected<fe::PipelineID, SolidColorPipelineError> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                            ResourceManager&                      resource_manager,
                                                                            IRenderer&                            renderer) {
            // TODO : fill this
            return {};
        }
    };
} // namespace fe
