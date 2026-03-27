/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for vert, frag and spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

void fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    resource::Shader this_shader{};



    auto ptr = storage.CreateResource<resource::Shader>(std::move(this_shader)); // does not need to store this pointer
}
