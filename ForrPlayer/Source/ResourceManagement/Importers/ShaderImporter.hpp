/*===============================================

    Forr Engine

    File : ShaderImporter.hpp
    Role : imports resources and their metadata. for spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceStorage.hpp"

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include "spirv_reflect.h"

namespace fe {
    struct ShaderImportContext {
        resource::Shader&            shader;
        ResourceStorage&             storage;
        const std::filesystem::path& resource_full_path;

        ShaderImportContext(resource::Shader& shader, ResourceStorage& storage, const std::filesystem::path& resource_full_path)
            : shader(shader), storage(storage), resource_full_path(resource_full_path) {}
        ~ShaderImportContext() = default;
    };

    class ShaderImporter {
    public:
        using Property     = resource::Shader::Property;
        using PropertyType = resource::Shader::Property::Type;

    public:
        ShaderImporter()  = default;
        ~ShaderImporter() = default;

        static fe::pointer<resource::Shader> Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);

    private:
        static bool loadSourceCode(ShaderImportContext& context);
    };
} // namespace fe
