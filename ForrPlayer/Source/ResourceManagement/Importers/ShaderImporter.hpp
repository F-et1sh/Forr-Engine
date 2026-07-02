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
    struct EntryPoint {
        Slang::ComPtr<slang::IEntryPoint>   entry_point{};
        resource::ShaderProgram::ShaderType shader_type{};

        EntryPoint()  = default;
        ~EntryPoint() = default;

        EntryPoint(slang::IEntryPoint* entry_point, resource::ShaderProgram::ShaderType shader_type)
            : entry_point(entry_point), shader_type(shader_type) {}
    };

    struct ShaderImportContext {
        static constexpr std::array<std::string_view, 3> entry_point_names{
            "vertexMain",
            "fragmentMain",
            "computeMain"
        };

        ResourceStorage&             storage;
        const std::filesystem::path& resource_full_path{};

        resource::ShaderProgram& shader_program;

        Slang::ComPtr<slang::ISession>       session{};
        Slang::ComPtr<slang::IComponentType> composed_program{};

        ShaderImportContext(ResourceStorage& storage, const std::filesystem::path& resource_full_path, resource::ShaderProgram& shader_program)
            : storage(storage), resource_full_path(resource_full_path), shader_program(shader_program) {}
        ~ShaderImportContext() = default;
    };

    class ShaderImporter {
    public:
        ShaderImporter()  = default;
        ~ShaderImporter() = default;

        static fe::pointer<resource::ShaderProgram> Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);

    private:
        static FORR_NODISCARD bool compile(ShaderImportContext& context);
        static FORR_NODISCARD bool reflect(ShaderImportContext& context);
        //static FORR_NODISCARD bool validate(ShaderImportContext& context);

        static void parseDescriptorTable(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedParameter& dst_parameter);
        static void parsePushConstant(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedPushConstants& dst_push_constants);

        static void parseMemberRecursive(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedDataNode* dst_reflected_data_node);
        static void parseMemberRecursive(ShaderImportContext& context, slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ReflectedDataNode* dst_reflected_data_node);

        static void parseTypeLayout(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedParameter& dst_parameter);

        static void parseElementTypeLayout(ShaderImportContext& context, slang::TypeLayoutReflection* element_type_layout, resource::ShaderProgram::ReflectedParameter& dst_parameter);

        static void decompile1(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedParameter& dst_parameter);
        static void decompile2(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedParameter& dst_parameter);
        static void decompile3(ShaderImportContext& context, slang::VariableLayoutReflection* variable_layout, resource::ShaderProgram::ReflectedParameter& dst_parameter);

        static void setupDescriptorType(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::DescriptorType& dst_descriptor_type);

        static void mapMatrix(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type);
        static void mapVector(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type);
        static void mapScalar(slang::TypeLayoutReflection* type_layout, resource::ShaderProgram::ValueType& type);
    };
} // namespace fe
