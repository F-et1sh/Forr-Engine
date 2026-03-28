/*===============================================

    Forr Engine

    File : MaterialImporter.hpp
    Role : imports resources and their metadata. for forr_material

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceStorage.hpp"

namespace fe {
    class MaterialImporter {
    public:
        MaterialImporter()  = default;
        ~MaterialImporter() = default;

        static void Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);
    };
}