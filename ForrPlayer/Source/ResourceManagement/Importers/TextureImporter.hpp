/*===============================================

    Forr Engine

    File : TextureImporter.hpp
    Role : imports resources and their metadata. for textures

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceStorage.hpp"

namespace fe {
    class TextureImporter {
    public:
        TextureImporter() = default;
        ~TextureImporter() = default;

        static void Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);
    };
}