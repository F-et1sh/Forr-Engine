/*===============================================

    Forr Engine

    File : ResourceCreator.cpp
    Role : creates engine-specific resources in the explorer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceCreator.hpp"

void fe::ResourceCreator::CreateDefaultResources() {
    this->createDefaultMaterials();
}

void fe::ResourceCreator::createDefaultMaterials() {
    this->createPBRMaterial();
}

void fe::ResourceCreator::createPBRMaterial() {
    std::filesystem::path shader_full_path = PATH.getDefaultShadersPath() / L"PBRMaterial" / L"PBRMaterial";
    auto                  shader_ptr       = m_Importer.ImportResource<resource::ShaderFileData>(shader_full_path.wstring() + PATH.getShaderExtension().wstring());

    if (!shader_ptr.is_valid()) {
        fe::logging::error("Failed to create default material : PBRMaterial.\nFailed to load file.\nPath : %s", shader_full_path.generic_string().c_str());
        return;
    }

    const auto& shader = *m_Storage.GetResource(shader_ptr);

    if (!shader.material_layout_ptrs.has_value()) {
        fe::logging::error("Failed to create default material : PBRMaterial.\nNo materials found.\nPath : %s", shader_full_path.generic_string().c_str());
        return;
    }

    const auto& material_layouts = shader.material_layout_ptrs.value();
    auto        it               = material_layouts.find("PBRMaterial");
    if (it == material_layouts.end()) {
        fe::logging::error("Failed to create default material : PBRMaterial.\nPBRMaterial structure not found.\nPath : %s", shader_full_path.generic_string().c_str());
    }

    const auto& material_layout = *m_Storage.GetResource(it->second);

    resource::Material material{};
    material.layout_ptr = it->second;
    material.buffer     = m_Storage.AllocateMaterialBufferSpan(material_layout.reflected_layout.size);
    //material.samplers = ... TODO : provide fallback textures

    m_Context.default_pbr_material_ptr = m_Storage.CreateResource(std::move(material));
}
