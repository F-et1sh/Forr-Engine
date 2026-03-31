/*===============================================

    Forr Engine

    File : TextureImporter.cpp
    Role : imports resources and their metadata. for textures

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "TextureImporter.hpp"

#include "stb_image.h"

using namespace fe::resource;

fe::pointer<Texture> fe::TextureImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    int            width{};
    int            height{};
    int            components{};
    unsigned char* bytes{};

    bytes = stbi_load(resource_full_path.string().c_str(), &width, &height, &components, 0);

    if (!bytes) {
        fe::logging::error("STBI -> Unified. Failed to load a texture\nPath : %s", components, resource_full_path.string().c_str());
        return {};
    }

    Texture::InternalFormat internal_format{};
    Texture::DataFormat     data_format{};

    // clang-format off
    switch (components) {
        case 4: internal_format = Texture::InternalFormat::RGBA8; data_format = Texture::DataFormat::RGBA; break;
        case 3: internal_format = Texture::InternalFormat::RGB8 ; data_format = Texture::DataFormat::RGB ; break;
        case 2: internal_format = Texture::InternalFormat::RG8  ; data_format = Texture::DataFormat::RG  ; break;
        case 1: internal_format = Texture::InternalFormat::R8   ; data_format = Texture::DataFormat::RED ; break;
        default:
            fe::logging::error("STBI -> Unified. Failed to load a texture. Unsupported number of components %i. Using RGBA8 for internal format and RGBA for data format as default\nPath : %s", 
                components, resource_full_path.string().c_str());

            internal_format = Texture::InternalFormat::RGBA8;
            data_format = Texture::DataFormat::RGBA;
    }
    // clang-format on

    fe::resource::Texture texture{};
    texture.width           = width;
    texture.height          = height;
    texture.components      = components;
    texture.internal_format = internal_format;
    texture.data_format     = data_format;

    size_t buffer_size = width * height * components;
    texture.bytes      = std::make_unique<unsigned char[]>(buffer_size);

    // bytes are already checked that it is not nullptr
    std::copy(bytes, bytes + buffer_size, texture.bytes.get());

    stbi_image_free(bytes); // can be freed after copying

    auto ptr = storage.CreateResource(std::move(texture));
    return ptr;
}
