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
    std::filesystem::path gltf_shader_path        = PATH.getDefaultShadersPath() / L"gLTF" / L"shader";
    auto                  default_gltf_shader_ptr = m_Importer.ImportResource<resource::ShaderProgram>(gltf_shader_path.wstring() + PATH.getShaderExtension().wstring());

    resource::Material gltf_material{};
    gltf_material.shader_program_ptr = default_gltf_shader_ptr;

    glm::vec3 color{ 0.76f, 0.67f, 0.52f };

    // TODO : m_Storage.AllocateMaterialBuffer();
    // TODO : m_Storage.AllocateMaterialSamplers();

    //gltf_material.buffer.resize(sizeof(glm::vec3));
    //memcpy(gltf_material.buffer.data(), &color, sizeof(glm::vec3));

    m_Context.default_gltf_material_ptr = m_Storage.CreateResource(std::move(gltf_material));
}
