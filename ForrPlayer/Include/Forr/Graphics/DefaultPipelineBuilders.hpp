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

    };

    class FORR_API PBREffectBuilder {
    public:
        static std::expected<PBRMaterialEffect, PBREffectErrors> Build(fe::pointer<resource::ShaderFileData> shader_file_data_ptr,
                                                                       fe::pointer<resource::Material>       material_ptr,
                                                                       ResourceManager&                      resource_manager) {
            PBRMaterialEffect pbr_material_effect{};

            resource_manager.GetResource(shader_file_data_ptr);
        }
    };
} // namespace fe
