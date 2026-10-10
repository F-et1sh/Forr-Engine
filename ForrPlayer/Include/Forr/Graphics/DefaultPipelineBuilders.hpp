/*===============================================

    Forr Engine

    File : DefaultPipelineBuilders.hpp
    Role : pipeline builders that provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Graphics/IRenderer.hpp"

namespace fe {
    enum class PBRPipelineErrorCodes : uint8_t {
        SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_PTR_WAS_INVALID,
        MATERIAL_LAYOUT_SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_LAYOUT_STRUCTURE_INDEX_WAS_INVALID,
        VERTEX_ENTRY_POINT_ABSENT,
        FRAGMENT_ENTRY_POINT_ABSENT,
        WRONG_NUMBER_OF_GENERIC_ARGUMENTS,
        NO_MATERIAL_INTERFACE_IN_GENERIC_ARGUMENT_CONSTRAINTS,
        UNKNOWN_GRAPHICS_BACKEND,
        FAILED_TO_CREATE_PIPELINE,
    };

    struct PBRPipelineError {
        using DetailedMessageVariants = std::variant<graphics::PipelineCreationErrors,
                                                     graphics::ParameterCreationErrors,
                                                     std::string>;

        PBRPipelineErrorCodes                  error_code{};
        std::optional<DetailedMessageVariants> detailed_message{}; // used only in specific cases

        PBRPipelineError(PBRPipelineErrorCodes error_code)
            : error_code(error_code) {}
        PBRPipelineError(PBRPipelineErrorCodes error_code, DetailedMessageVariants detailed_message)
            : error_code(error_code), detailed_message(std::move(detailed_message)) {}
    };

    class PBRPipelineBuilder {
    public:
        static std::expected<fe::graphics::PipelineHandle, PBRPipelineError> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                                   fe::pointer<resource::Material>       material_ptr,
                                                                                   ResourceManager&                      resource_manager,
                                                                                   IRenderer&                            renderer) {
            /* declare common shader resources */
            const static fe::hashed_string vertex_entry_point_name{ "vertexMain" };
            const static fe::hashed_string fragment_entry_point_name{ "fragmentMain" };
            const static fe::hashed_string material_interface_name{ "IMaterial" };
            const static fe::hashed_string materials_raw_data_name{ "g_MaterialsRawData" };
            const static fe::hashed_string model_matrices_name{ "g_ModelMatrices" };
            const static fe::hashed_string global_data_name{ "g_GlobalData" };
            const static fe::hashed_string push_constants_name{ "push_constants" };
            const static fe::hashed_string unspecialized_buffer_name{ "TBuffer" };
            const static fe::hashed_string opengl_buffer_name{ "OpenGLBuffer" };
            const static fe::hashed_string vulkan_buffer_name{ "VulkanBuffer" };

            constexpr static size_t entry_point_generic_argument_count = 1;

            // load shader file data and check for error
            auto shader_file_data_raw_pointer = resource_manager.GetResource(shader_file_data_ptr);
            if (!shader_file_data_raw_pointer)
                return std::unexpected{ PBRPipelineErrorCodes::SHADER_FILE_DATA_PTR_WAS_INVALID };

            // load material and check for error
            auto material_raw_pointer = resource_manager.GetResource(material_ptr);
            if (!material_raw_pointer)
                return std::unexpected{ PBRPipelineErrorCodes::MATERIAL_PTR_WAS_INVALID };

            /* translate 'optionals' to 'T&' */
            auto& shader_file_data = *shader_file_data_raw_pointer;
            auto& material         = *material_raw_pointer;

            // pre-create pipeline desc
            graphics::PipelineDesc pipeline_desc{
                .pipeline_flags        = material.pipeline_flags_override,
                .shader_file_data_ptrs = { shader_file_data_ptr },
                .entry_points          = { vertex_entry_point_name, fragment_entry_point_name },
                .descriptor_sets       = { materials_raw_data_name, model_matrices_name, global_data_name },
                .push_constants        = { push_constants_name },
            };

            // this is needed to write directly into 'pipeline_desc' without extra copyings
            auto& specialization = pipeline_desc.specialization.emplace();

            // shader file data where we gonna search for material's structure
            std::reference_wrapper<resource::ShaderFileData> shader_file_data_to_find_material_structure{ shader_file_data };

            // load material's shader file data if they are not from the same file
            if (material.layout_key.shader_file_data != shader_file_data_ptr) {
                auto material_shader_file_data_raw_pointer = resource_manager.GetResource(material.layout_key.shader_file_data);
                if (!material_shader_file_data_raw_pointer)
                    return std::unexpected{ PBRPipelineErrorCodes::MATERIAL_LAYOUT_SHADER_FILE_DATA_PTR_WAS_INVALID };

                pipeline_desc.shader_file_data_ptrs.emplace_back(material.layout_key.shader_file_data);
                shader_file_data_to_find_material_structure = *material_shader_file_data_raw_pointer;
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
                // make sure that 'shader_file_data' contains declarated entry points
                auto entry_point_it = std::ranges::find_if(shader_file_data.entry_points, [&entry_point](const auto& e) -> bool {
                    return e.name == entry_point;
                });

                if (entry_point_it == shader_file_data.entry_points.end()) {
                    return std::unexpected{ entry_point == vertex_entry_point_name ? PBRPipelineErrorCodes::VERTEX_ENTRY_POINT_ABSENT
                                                                                   : PBRPipelineErrorCodes::FRAGMENT_ENTRY_POINT_ABSENT };
                }
                
                // make sure that this entry point has only one generic argument to specialize
                size_t generic_arguments_count = entry_point_it->generic_arguments.size();
                if (generic_arguments_count != entry_point_generic_argument_count) {
                    std::string error_message{};
                    error_message.reserve(50 + entry_point_it->name.size());
                    error_message = std::string{ "Entry point name : " } + entry_point_it->name.c_str() +
                                    std::string{ "\nArguments count : " } + std::to_string(generic_arguments_count);

                    return std::unexpected{ PBRPipelineError{ PBRPipelineErrorCodes::WRONG_NUMBER_OF_GENERIC_ARGUMENTS, error_message } };
                }

                // make sure that this generic argument can be specialized with 'material_interface_name'
                const auto& generic_argument = entry_point_it->generic_arguments.back();
                auto        constaint_it     = std::ranges::find_if(generic_argument, [](const auto& e) -> bool {
                    return e == material_interface_name;
                });

                if (constaint_it == generic_argument.end()) {
                    std::string error_message{};
                    error_message.reserve(10 * generic_argument.size() + 50 + entry_point_it->name.size());
                    error_message = std::string{ "Entry point name : " } + entry_point_it->name.c_str() +
                                    std::string{ "\nConstaints : " };

                    for (const auto& constraints : generic_argument) {
                        error_message.append_range(std::string{ "\n" + constraints });
                    }

                    return std::unexpected{ PBRPipelineError{ PBRPipelineErrorCodes::NO_MATERIAL_INTERFACE_IN_GENERIC_ARGUMENT_CONSTRAINTS, error_message } };
                }

                specialization.entry_points.emplace_back(shader::EntryPointSpecialization{
                    .name      = entry_point,
                    .arguments = { shader::SpecializationArgument{
                        .name  = material_interface_name,
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

    struct SolidColorPipelineError {
        using DetailedMessageVariants = std::variant<SolidColorPipelineErrorCodes,
                                                     graphics::ParameterCreationErrors,
                                                     fe::hashed_string>;

        SolidColorPipelineErrorCodes           error_code{};
        std::optional<DetailedMessageVariants> detailed_message{}; // used only in specific cases

        SolidColorPipelineError(SolidColorPipelineErrorCodes error_code)
            : error_code(error_code) {}
        SolidColorPipelineError(SolidColorPipelineErrorCodes error_code, DetailedMessageVariants detailed_message)
            : error_code(error_code), detailed_message(std::move(detailed_message)) {}
    };

    class SolidColorPipelineBuilder {
        static std::expected<fe::graphics::PipelineHandle, SolidColorPipelineError> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                                          ResourceManager&                      resource_manager,
                                                                                          IRenderer&                            renderer) {
            // TODO : fill this
            return {};
        }
    };
} // namespace fe
