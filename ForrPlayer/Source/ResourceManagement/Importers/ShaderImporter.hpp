/*===============================================

    Forr Engine

    File : ShaderImporter.hpp
    Role : imports resources and their metadata. for vert, frag and spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceStorage.hpp"

namespace fe {
    class ShaderImporter {
    public:
        ShaderImporter()  = default;
        ~ShaderImporter() = default;

        static void Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);
    };
}