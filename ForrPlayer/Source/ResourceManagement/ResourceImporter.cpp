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
#include "Importers/ShaderImporter.hpp"
#include "Importers/MaterialImporter.hpp"

#define IMPORT_INSTANCE(T, T_IMPORTER)                                                                     \
    template <>                                                                                            \
    fe::pointer<T> fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_full_path) { \
        return T_IMPORTER::Import(m_Storage, resource_full_path);                                          \
    }                                                                                                      \
    template fe::pointer<T> fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_full_path);

void fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_full_path) {
    std::filesystem::path extension = resource_full_path.extension();

    // TODO : add metadata parsing ( .forr_meta )

    if (extension == ".png") {
        TextureImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".gltf" || extension == ".glb") {
        GLTFImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".spv") {
        ShaderImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".forr_material") {
        MaterialImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".vert" || extension == ".frag") {
        ShaderImporter::Import(m_Storage, resource_full_path);
    }
}

IMPORT_INSTANCE(fe::resource::Texture, fe::TextureImporter)
IMPORT_INSTANCE(fe::resource::Model, fe::GLTFImporter)
IMPORT_INSTANCE(fe::resource::Shader, fe::ShaderImporter)
IMPORT_INSTANCE(fe::resource::Material, fe::MaterialImporter)

#undef IMPORT_INSTANCE
