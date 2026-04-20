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
#include "Graphics/Shaders/ShaderCompiler.hpp"

using namespace fe::resource;

fe::pointer<fe::resource::Shader> fe::ShaderImporter::Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path) {
    Shader shader{};

    std::ifstream file(resource_full_path, std::ios::binary | std::ios::ate);
    if (!file.good()) {
        fe::logging::error("File -> Unified. Failed to open shader file\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    std::streampos file_size{};

    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string source_code{};

    source_code.resize(file_size);
    file.read((char*) &source_code[0], file_size);

    if (resource_full_path.extension() == PATH.getVertexShaderExtension()) {
        shader.type = Shader::Type::VERTEX;
    }
    else if (resource_full_path.extension() == PATH.getFragmentShaderExtension()) {
        shader.type = Shader::Type::FRAGMENT;
    }
    else {
        fe::logging::error("File -> Unified. Unknown shader file extension\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    const auto& resource_management_context = storage.GetContext();
    ShaderCompiler::Compile(shader.source_code, source_code, shader.type, resource_management_context.graphics_backend);

    bool is_valid{};
    ShaderReflector::Reflect(shader, is_valid);

    if (!is_valid) {
        fe::logging::error("Shader hasn't enough members ( check SceneData SSBO )\nPath : %s", resource_full_path.string().c_str());
        return {};
    }

    auto ptr = storage.CreateResource(std::move(shader));
    return ptr;
}
