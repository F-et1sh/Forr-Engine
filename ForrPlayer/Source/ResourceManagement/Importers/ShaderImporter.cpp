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

    shader_file_data.full_path = resource_full_path.generic_string().c_str();

    if (!parser.ExtractSerializedData(shader_file_data.slang_serialized_data)) {
        fe::logging::error("Slang -> Unified. Failed to extract serialized data\nPath : %s", resource_full_path.generic_string().c_str());
        return {};
    }

    if (!parser.ComposeProgram()) {
        fe::logging::error("Slang -> Unified. Failed to compose the program\nPath : %s", resource_full_path.generic_string().c_str());
        return {};
    }

    auto ptr = storage.CreateResource(std::move(shader_file_data));
    // after moving 'shader_file_data' into the storage, we can't use that value again
    auto& this_shader_file_data = *storage.GetResource(ptr);

    shader::ReflectedDescriptorsLayout       descriptors_layout{};
    fe::pointer<resource::DescriptorsLayout> descriptors_layout_ptr{};

    if (parser.ReflectDescriptors(descriptors_layout)) {
        descriptors_layout_ptr                       = storage.CreateResource(resource::DescriptorsLayout{ std::move(descriptors_layout), ptr });
        this_shader_file_data.descriptors_layout_ptr = descriptors_layout_ptr;
    }

    if (parser.IsPipeline()) {
        resource::ShaderProgram shader_program{};

        if (descriptors_layout_ptr.is_valid()) {
            shader_program.descriptors_layout_ptr = descriptors_layout_ptr;
        }
        else {
            shader_program.descriptors_layout_ptr = std::nullopt;
        }

        shader_program.shader_file_data_ptr = ptr;

        auto pipeline_ptr                        = storage.CreateResource(std::move(shader_program));
        this_shader_file_data.shader_program_ptr = pipeline_ptr;
    }

    std::unordered_map<fe::hashed_string, shader::ReflectedStructureLayout> material_layouts{};
    if (parser.ReflectMaterials(material_layouts)) {

        auto& material_layout_ptrs = this_shader_file_data.material_layout_ptrs.emplace();
        material_layout_ptrs.reserve(material_layouts.size());

        for (auto& [material_name, material_layout] : material_layouts) {

            auto material_layout_ptr = storage.CreateResource(std::move(resource::MaterialLayout{ std::move(material_layout), ptr }));
            material_layout_ptrs.emplace(material_name, material_layout_ptr);
        }
    }

    return ptr;
}
