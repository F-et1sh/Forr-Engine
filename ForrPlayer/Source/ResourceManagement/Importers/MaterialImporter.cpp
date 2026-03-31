/*===============================================

    Forr Engine

    File : MaterialImporter.cpp
    Role : imports resources and their metadata. for vert, frag and spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "MaterialImporter.hpp"

#include <fstream>

fe::pointer<fe::resource::Material> fe::MaterialImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    resource::Material this_material{};

    auto ptr = storage.CreateResource<resource::Material>(std::move(this_material));
    return ptr;
}
