/*===============================================

    Forr Engine

    File : ShaderImporter.hpp
    Role : imports resources and their metadata. for spv

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceStorage.hpp"

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

namespace fe {
    class ShaderImporter {
    public:
        ShaderImporter()  = default;
        ~ShaderImporter() = default;

        static fe::pointer<resource::ShaderProgram> Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);

    private:
        static FORR_NODISCARD slang::IComponentType* compile(fe::resource::ShaderProgram& shader_program,
                                                             ResourceStorage&             storage,
                                                             const std::filesystem::path& resource_full_path);
        static FORR_NODISCARD bool                   reflect(fe::resource::ShaderProgram& shader_program, slang::IComponentType* composed_program);

        static resource::ShaderProgram::ResourceClass to_resource_class(slang::TypeReflection::Kind kind);
        static resource::ShaderProgram::ValueType     to_value_type(slang::TypeReflection* type);
    };
} // namespace fe
