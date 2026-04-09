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

void fe::ResourceCreator::CreateMaterial(fe::pointer<resource::Material> pointer, const std::filesystem::path& resource_full_path) {
    resource::Material& material = *m_Storage.GetResource(pointer);

    std::ofstream file(resource_full_path);
    if (!file.good()) {
        fe::logging::error("Unified -> %s. Failed create material\nPath : %s",
                           resource_full_path.extension().string().c_str(),
                           resource_full_path.string().c_str());
        return;
    }
}

void fe::ResourceCreator::createDefaultMaterials() {
    resource::Shader gltf_vertex_shader{};
    gltf_vertex_shader.source_code = {};

    //resource::Material gltf_material{};
    //gltf_material.
}
