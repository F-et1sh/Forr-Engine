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
    std::filesystem::path gltf_shader_path        = PATH.getDefaultShadersPath() / L"PBRMaterial" / L"shader2";
    auto                  default_gltf_shader_ptr = m_Importer.ImportResource<resource::ShaderReflectedData>(gltf_shader_path.wstring() + PATH.getShaderExtension().wstring());

    const auto& default_gltf_shader = *m_Storage.GetResource(default_gltf_shader_ptr);

    if (!default_gltf_shader.material_layout_ptrs.has_value()) {
        fe::logging::fatal("Failed to create default material : PBRMaterial");
        return;
    }

    const auto& material_layouts = default_gltf_shader.material_layout_ptrs.value();
    auto it = material_layouts.find("PBRMaterial");
    if (it == material_layouts.end()) {
        fe::logging::fatal("Failed to create default material : PBRMaterial");
        return;
    }

    resource::Material gltf_material{};
    gltf_material.layout_ptr = it->second;

    glm::vec3 color{ 0.76f, 0.67f, 0.52f };

    // TODO : m_Storage.AllocateMaterialBuffer();
    // TODO : m_Storage.AllocateMaterialSamplers();

    //gltf_material.buffer.resize(sizeof(glm::vec3));
    //memcpy(gltf_material.buffer.data(), &color, sizeof(glm::vec3));

    m_Context.default_gltf_material_ptr = m_Storage.CreateResource(std::move(gltf_material));
}
