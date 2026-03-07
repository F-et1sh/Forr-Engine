/*===============================================

    Forr Engine

    File : ResourceImporter.cpp
    Role : imports resources and their metadata

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ResourceManagement/ResourceImporter.hpp"

#include "stb_image.h" // temp

void fe::ResourceImporter::StoreResource(const std::filesystem::path& resource_relative_path) {
    std::filesystem::path resource_full_path = PATH.getResourcesPath() / resource_relative_path;
    std::filesystem::path extension          = resource_full_path.extension();

    if (extension == ".png") {
        this->StoreTexture(resource_full_path);
    }
}

void fe::ResourceImporter::StoreTexture(const std::filesystem::path& resource_full_path) {
    std::filesystem::path metadata_full_path = resource_full_path.wstring() + L".fs"; // TODO : write this to some desc

    int            width{};
    int            height{};
    int            components{};
    unsigned char* bytes{};

    bytes = stbi_load(resource_full_path.string().c_str(), &width, &height, &components, 0);

    if (!bytes) {
        fe::logging::error("Failed to load a texture\nPath : %s", components, resource_full_path.string().c_str());
        return;
    }

    TextureInternalFormat internal_format{};
    TextureDataFormat     data_format{};

    // clang-format off
    switch (components) {
        case 4: internal_format = TextureInternalFormat::RGBA8; data_format = TextureDataFormat::RGBA; break;
        case 3: internal_format = TextureInternalFormat::RGB8; data_format = TextureDataFormat::RGB; break;
        case 2: internal_format = TextureInternalFormat::RG8; data_format = TextureDataFormat::RG; break;
        case 1: internal_format = TextureInternalFormat::R8; data_format = TextureDataFormat::RED; break;
        default:
            fe::logging::error("Failed to load a texture. Wrong number of components : %i\nPath : %s", components, resource_full_path.string().c_str());
            return;
            break;
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

    auto ptr = m_Storage.m_Textures.create(std::move(texture));
    m_Storage.m_TexturePointers.emplace_back(ptr);
}
