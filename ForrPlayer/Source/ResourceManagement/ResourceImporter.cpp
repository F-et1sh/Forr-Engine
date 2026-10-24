/*===============================================

    Forr Engine

    File : ResourceImporter.cpp
    Role : imports resources and their metadata

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/ResourceManagement/ResourceImporter.hpp"

#include "Importers/TextureImporter.hpp"
#include "Importers/GLTFImporter.hpp"
#include "Importers/ShaderImporter.hpp"
#include "Importers/MaterialImporter.hpp"

#include "Forr/Core/path.hpp"

#define IMPORTER_INSTANCE(T, T_IMPORTER)                                                                   \
    template <>                                                                                            \
    fe::pointer<T> fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_full_path) { \
        return T_IMPORTER::Import(m_Storage, resource_full_path);                                          \
    }                                                                                                      \
    template FORR_API fe::pointer<T> fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_full_path);

void fe::ResourceImporter::ImportResource(const std::filesystem::path& resource_full_path) {
    std::filesystem::path extension = resource_full_path.extension();

    // TODO : add metadata parsing ( .forr_meta )

    if (extension == ".png") {
        std::ignore = TextureImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension == ".gltf" || extension == ".glb") {
        std::ignore = GLTFImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension.generic_wstring() == PATH.getMaterialExtension()) {
        std::ignore = MaterialImporter::Import(m_Storage, resource_full_path);
    }
    else if (extension.generic_wstring() == PATH.getShaderExtension()) {
        std::ignore = ShaderImporter::Import(m_Storage, resource_full_path);
    }
}

IMPORTER_INSTANCE(fe::resource::Texture, fe::TextureImporter)
IMPORTER_INSTANCE(fe::resource::Model, fe::GLTFImporter)
IMPORTER_INSTANCE(fe::resource::ShaderFileData, fe::ShaderImporter)
IMPORTER_INSTANCE(fe::resource::Material, fe::MaterialImporter)

#undef IMPORTER_INSTANCE
