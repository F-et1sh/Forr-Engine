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
    struct ShaderImportContext {
        static constexpr std::array<std::string_view, 3> entry_point_names{
            "vertexMain",
            "fragmentMain",
            "computeMain"
        };

        ResourceStorage&             storage;
        const std::filesystem::path& resource_full_path{};

        resource::ShaderProgram&       shader_program;
        resource::ShaderReflectedData& shader_reflected_data;

        Slang::ComPtr<slang::ISession> session{};
        slang::ProgramLayout*          root_layout{};

        ShaderImportContext(ResourceStorage& storage, const std::filesystem::path& resource_full_path, resource::ShaderProgram& shader_program, resource::ShaderReflectedData& shader_reflected_data)
            : storage(storage), resource_full_path(resource_full_path), shader_program(shader_program), shader_reflected_data(shader_reflected_data) {}
        ~ShaderImportContext() = default;
    };

    class ShaderImporter {
    public:
        ShaderImporter()  = default;
        ~ShaderImporter() = default;

        static fe::pointer<resource::ShaderReflectedData> Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);

    private:
        static FORR_NODISCARD bool compile(ShaderImportContext& context);
        static FORR_NODISCARD bool reflect(ShaderImportContext& context);
        static FORR_NODISCARD bool validate(ShaderImportContext& context);

        static void checkAndPrintProblem(const std::string& field_name, auto changed_to, auto should_be, fe::logging::Severity severity);
        static void checkDataNode(ShaderImportContext& context, const std::string& field_name, const shader::ReflectedDataNode* data_node, const shader::ReflectedDataNode* expected_data_node);

        static void parseDescriptorTable(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptor& dst_descriptor);
        static void parsePushConstant(slang::VariableLayoutReflection* variable_layout, shader::ReflectedPushConstants& dst_push_constants);

        static void parseMemberRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDataNode* dst_reflected_data_node);
        static void parseMemberRecursive(slang::TypeLayoutReflection* type_layout, shader::ReflectedDataNode* dst_reflected_data_node);

        static void mapMatrix(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        static void mapVector(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        static void mapScalar(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
    };
} // namespace fe
