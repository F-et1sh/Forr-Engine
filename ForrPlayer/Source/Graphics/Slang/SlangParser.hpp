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

#include "ResourceManagement/ResourceManager.hpp"

namespace fe {
    class SlangParser {
    private:
        static constexpr std::array<std::string_view, 3> ENTRY_POINT_NAMES{
            "vertexMain",
            "fragmentMain",
            "computeMain"
        };

        enum class SpecializationErrors { // first time of using 'std::expected'
            NO_PARAMETERS,
            UNSUPPORTED_GRAPHICS_BACKEND,
            SPECIALIZATION_FAILED
        };

    public:
        // this will use 'PATH.getShadersPath().generic_string().c_str()' if you leave argument 'search_paths' as null
        SlangParser(std::span<const char*> full_search_paths = {});
        ~SlangParser() = default;

        FORR_CLASS_MOVABLE(SlangParser)
        FORR_CLASS_NONCOPYABLE(SlangParser)

        bool LoadFromFile(const std::filesystem::path& resource_full_path);

        bool ExtractSerializedData(std::vector<uint8_t>& dst_vector);

        // pass the graphics backend to specialize the shader program
        bool ComposeProgram(GraphicsBackend graphics_backend, bool do_all = false);

        // returns 'true', if actually found anything and 'false', if the argument is not changed
        bool ReflectDescriptors(shader::ReflectedDescriptorsLayout& descriptors_layout);

        // returns 'true', if the shader file contains vertexMain(), fragmentMain() or computeMain() and otherwise - 'false'
        bool IsPipeline();

        // returns 'true', if actually found anything and 'false', if the argument is not changed
        bool ReflectMaterials(std::unordered_map<fe::hashed_string, shader::ReflectedStructureLayout>& material_layouts);

        // TODO : remove this and create 'unified' version - it mustn't just combine shader program and material,
        //  but specialize shader program, material and descriptors in any way possible
        shader::SourceCodeStorage CombineAndCompileShader(const resource::ShaderProgram& shader_program,
                                                          const resource::Material&      material,
                                                          ResourceManager&               resource_manager);

    private:
        std::expected<Slang::ComPtr<slang::IComponentType>, SpecializationErrors> specializeGraphicsBackend(slang::IComponentType* component_type, GraphicsBackend graphics_backend);

        bool parseDescriptorRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptorsLayout& descriptors_layout);

        void parseDescriptorTable(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptor& dst_descriptor);
        void parsePushConstant(slang::VariableLayoutReflection* variable_layout, shader::ReflectedPushConstants& dst_push_constants);

        void parseMemberRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDataNode* dst_reflected_data_node);
        void parseMemberRecursive(slang::TypeLayoutReflection* type_layout, shader::ReflectedDataNode* dst_reflected_data_node);

        void mapMatrix(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        void mapVector(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        void mapScalar(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);

        Slang::ComPtr<slang::IModule> deserializeModule(fe::pointer<resource::ShaderFileData> shader_file_data_ptr, ResourceManager& resource_manager);

    private:
        Slang::ComPtr<slang::ISession>       m_Session{};
        Slang::ComPtr<slang::IModule>        m_Module{};
        Slang::ComPtr<slang::IComponentType> m_ComposedProgram{};

        // static cache
    };
} // namespace fe
