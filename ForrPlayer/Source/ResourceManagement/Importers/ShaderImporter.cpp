/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for vert, frag and spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

void fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    resource::Shader this_shader{};

    std::ifstream file(resource_full_path);
    if (!file.good()) {
        fe::logging::error("%s -> Unified. Failed to open shader file\nPath : %s",
                           resource_full_path.extension().string().c_str(),
                           resource_full_path.string().c_str());
        return;
    }

    this_shader.source = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()).c_str();

    auto ptr = storage.CreateResource<resource::Shader>(std::move(this_shader)); // does not need to store this pointer
}
