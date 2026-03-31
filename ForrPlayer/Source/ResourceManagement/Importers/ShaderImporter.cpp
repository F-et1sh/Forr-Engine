/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for vert, frag and glsl

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include "spirv_reflect.h"

fe::pointer<fe::resource::Shader> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    fe::resource::Shader shader{};

    std::ifstream file(resource_full_path);
    if (!file.good()) {
        fe::logging::error("Failed to open shader file\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    shader.source_code = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()).c_str();

    //shader.layout.properties

    auto ptr = storage.CreateResource(std::move(shader));
    return ptr;
}
