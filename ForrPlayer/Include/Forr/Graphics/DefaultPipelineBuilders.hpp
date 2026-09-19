/*===============================================

    Forr Engine

    File : DefaultPipelineBuilders.hpp
    Role : pipeline builders that provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"

namespace fe {
    // 'effects' are structures that contain everything that was built by the builder
    struct FORR_API PBRMaterialEffect {
        fe::PipelineID pipeline_id{};

        fe::ParameterID model_matrices_parameter_id{};
        fe::ParameterID global_data_parameter_id{};
        fe::ParameterID materials_raw_data_parameter_id{};
    };

    enum class PBREffectErrors : uint8_t {
        SHADER_FILE_DATA_PTR_WAS_INVALID,
        MATERIAL_PTR_WAS_INVALID
    };

    class FORR_API PBREffectBuilder {
    public:
        static std::expected<PBRMaterialEffect, PBREffectErrors> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                       fe::pointer<resource::Material>       material_ptr,
                                                                       ResourceManager&                      resource_manager) {
            auto shader_file_data_optional = resource_manager.GetResource(shader_file_data_ptr);
            if (!shader_file_data_optional.has_value())
                return std::unexpected{ PBREffectErrors::SHADER_FILE_DATA_PTR_WAS_INVALID };

            auto material_ontional = resource_manager.GetResource(material_ptr);
            if (!material_ontional.has_value())
                return std::unexpected{ PBREffectErrors::MATERIAL_PTR_WAS_INVALID };

            auto& shader_file_data = shader_file_data_optional.value();
            auto& material = material_ontional.value();
            
            PBRMaterialEffect pbr_material_effect{};

            material.pipeline_flags_override;
        }
    };
} // namespace fe
