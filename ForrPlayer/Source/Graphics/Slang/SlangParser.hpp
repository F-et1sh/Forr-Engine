/*===============================================

    Forr Engine

    File : SlangParser.hpp
    Role : this class compiles and reflects Slang shaders.
        ShaderImporter ( primary processing ) - reflect shader data and save serialized one.
        OpenGLResourceManager/VulkanResourceManager ( secondary processing ) - merge shader's and material's serialized
            data and finally compile the shader

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "slang.h"
#include "slang-com-ptr.h"
#include "slang-com-helper.h"

#include "ResourceManagement/Resources.hpp"

namespace fe {
    struct EntryPoint {
        Slang::ComPtr<slang::IEntryPoint> entry_point{};
        shader::StageBits                 shader_type{};

        EntryPoint() = default;
        EntryPoint(slang::IEntryPoint* entry_point, shader::StageBits shader_type)
            : entry_point(entry_point), shader_type(shader_type) {}

        FORR_CLASS_MOVABLE(EntryPoint)
        FORR_CLASS_NONCOPYABLE(EntryPoint)
    };

    class SlangParser {
    private:
        static constexpr std::array<std::string_view, 3> ENTRY_POINT_NAMES{
            "vertexMain",
            "fragmentMain",
            "computeMain"
        };

    public:
        SlangParser(GraphicsBackend graphics_backend);
        ~SlangParser() = default;

        FORR_CLASS_MOVABLE(SlangParser)
        FORR_CLASS_NONCOPYABLE(SlangParser)

        // primary processing : load .slang file, save its reflected and serialized data for the secondary processing
        bool LoadFromFileAndReflect(const std::filesystem::path& resource_full_path, resource::ShaderReflectedData& shader_reflected_data);

    private:
        std::vector<EntryPoint> findEntryPoints(std::vector<slang::IComponentType*>& component_types);
        void                    reflect(resource::ShaderReflectedData& shader_reflected_data);

        // returns 'true', if actually found anything and 'false', if the argument is not changed
        bool reflectPipeline(shader::ReflectedPipelineLayout& pipeline_layout);
        // returns 'true', if actually found anything and 'false', if the argument is not changed
        bool reflectMaterial(shader::ReflectedMaterialLayout& material_layout);

        bool parseDeclarationRecursive(slang::DeclReflection* member);
        void parseVariableRecursive(slang::VariableReflection* member);

        void parseDescriptorTable(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptor& dst_descriptor);
        void parsePushConstant(slang::VariableLayoutReflection* variable_layout, shader::ReflectedPushConstants& dst_push_constants);

        void parseMemberRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDataNode* dst_reflected_data_node);
        void parseMemberRecursive(slang::TypeLayoutReflection* type_layout, shader::ReflectedDataNode* dst_reflected_data_node);

        void mapMatrix(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        void mapVector(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        void mapScalar(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);

    private:
        GraphicsBackend                      m_GraphicsBackend{};
        Slang::ComPtr<slang::ISession>       m_Session{};
        slang::IModule*                      m_Module{};
        Slang::ComPtr<slang::IComponentType> m_CompusedProgram{};
    };
} // namespace fe
