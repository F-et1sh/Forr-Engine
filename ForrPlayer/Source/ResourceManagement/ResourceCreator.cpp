/*===============================================

    Forr Engine

    File : ResourceCreator.cpp
    Role : creates engine-specific resources in the explorer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceCreator.hpp"

#include "Graphics/Shaders/ShaderCompiler.hpp"

void fe::ResourceCreator::CreateDefaultResources() {
    this->createDefaultShaders();
    this->createDefaultMaterials();
}

void fe::ResourceCreator::createDefaultShaders() {
    std::filesystem::path gltf_shaders_path = PATH.getDefaultShadersPath() / L"gLTF" / L"shader";

    m_Context.default_gltf_vertex_shader_ptr   = m_Importer.ImportResource<resource::Shader>(gltf_shaders_path.wstring() + PATH.getVertexShaderExtension().wstring());
    m_Context.default_gltf_fragment_shader_ptr = m_Importer.ImportResource<resource::Shader>(gltf_shaders_path.wstring() + PATH.getFragmentShaderExtension().wstring());
}

void fe::ResourceCreator::createDefaultMaterials() {
    resource::Material gltf_material{};
    gltf_material.vertex_shader_ptr   = m_Context.default_gltf_vertex_shader_ptr;
    gltf_material.fragment_shader_ptr = m_Context.default_gltf_fragment_shader_ptr;
    gltf_material.color               = { 0.6f, 0.7f, 0.1f };

    m_Context.default_gltf_material_ptr = m_Storage.CreateResource(std::move(gltf_material));
}
