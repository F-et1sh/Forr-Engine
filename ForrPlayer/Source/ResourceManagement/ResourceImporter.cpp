/*===============================================

    Forr Engine

    File : ResourceImporter.cpp
    Role : imports resources and their metadata

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceImporter.hpp"

#include "Importers/TextureImporter.hpp"
#include "Importers/GLTFImporter.hpp"
#include "Importers/MaterialImporter.hpp"

void fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_relative_path) {
    std::filesystem::path resource_full_path = PATH.getResourcesPath() / resource_relative_path;
    std::filesystem::path extension          = resource_full_path.extension();

    // TODO : add metadata parsing ( .forr_meta )

    if (extension == ".png") {
        TextureImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".gltf" || extension == ".glb") {
        GLTFImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".forr_material") {
        MaterialImporter::Import(m_Storage, resource_full_path);
    }
}
