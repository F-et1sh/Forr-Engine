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

        bool LoadFromFile(const std::filesystem::path& resource_full_path);

        bool ExtractSerializedData(std::vector<uint8_t>& dst_vector);

        bool ComposeProgram();

        // returns 'true', if actually found anything and 'false', if the argument is not changed
        bool ReflectPipeline(shader::ReflectedPipelineLayout& pipeline_layout);

        // returns 'true', if actually found anything and 'false', if the argument is not changed
        bool ReflectMaterials(std::unordered_map<fe::hashed_string, shader::ReflectedMaterialLayout>& material_layouts);

    private:
        std::vector<EntryPoint> findEntryPoints(std::vector<slang::IComponentType*>& component_types);

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
