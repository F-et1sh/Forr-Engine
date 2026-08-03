/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for Slang

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include "Graphics/Slang/SlangParser.hpp"

fe::pointer<fe::resource::ShaderFileData> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    resource::ShaderFileData shader_file_data{};

    SlangParser parser{ storage.GetContext().graphics_backend };
    if (!parser.LoadFromFile(resource_full_path)) {
        fe::logging::error("Slang -> Unified. Failed to load a shader\nPath : %s", resource_full_path.generic_string().c_str());
        return {};
    }

    if (!parser.ExtractSerializedData(shader_file_data.slang_serialized_data)) {
        fe::logging::error("Slang -> Unified. Failed to extract serialized data\nPath : %s", resource_full_path.generic_string().c_str());
        return {};
    }

    if (!parser.ComposeProgram()) {
        fe::logging::error("Slang -> Unified. Failed to compose the program\nPath : %s", resource_full_path.generic_string().c_str());
        return {};
    }

    shader::ReflectedPipelineLayout pipeline_layout{};
    if (parser.ReflectPipeline(pipeline_layout)) {

        resource::ShaderProgram shader_program{};
        shader_program.reflected_layout = pipeline_layout;

        auto pipeline_ptr                  = storage.CreateResource(std::move(shader_program));
        shader_file_data.pipeline_ptr = pipeline_ptr;
    }

    std::unordered_map<fe::hashed_string, shader::ReflectedMaterialLayout> material_layouts{};
    if (parser.ReflectMaterials(material_layouts)) {

        auto& material_layout_ptrs = shader_file_data.material_layout_ptrs.emplace();
        material_layout_ptrs.reserve(material_layouts.size());

        for (auto& [material_name, material_layout] : material_layouts) {

            auto material_layout_ptr = storage.CreateResource(std::move(resource::MaterialLayout{ std::move(material_layout) }));
            material_layout_ptrs.emplace(material_name, material_layout_ptr);
        }
    }

    auto ptr = storage.CreateResource(std::move(shader_file_data));
    return ptr;
}
