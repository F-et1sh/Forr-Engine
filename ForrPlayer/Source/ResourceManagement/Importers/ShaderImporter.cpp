/*===============================================

    Forr Engine

    File : ShaderImporter.cpp
    Role : imports resources and their metadata. for spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderImporter.hpp"

#include <fstream>

#include "Graphics/Shaders/ShaderReflector.hpp"

using namespace fe::resource;

fe::pointer<fe::resource::Shader> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    Shader shader{};

    std::ifstream file(resource_full_path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        fe::logging::error("SPR-V -> Unified. Failed to open shader file\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    std::streampos file_size{};

    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    shader.source_code.resize(file_size);
    file.read((char*) &shader.source_code[0], file_size);

    ShaderReflector::Reflect(shader);

    if (resource_full_path.extension() == ".vert") // TODO : rewrite this
        shader.type = Shader::Type::VERTEX;
    else
        shader.type = Shader::Type::FRAGMENT;

    auto ptr = storage.CreateResource(std::move(shader));
    return ptr;
}
