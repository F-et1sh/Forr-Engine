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
    public:
        enum class ShaderBuildErrors : uint8_t {
            COMPOSITION_FAILED,
            FAILED_TO_SPECIALIZE_GLOBAL_PARAMETERS
        };

        enum class ShaderFileDataErrors : uint8_t {
            FAILED_TO_LOAD_SLANG_MODULE,
            FAILED_TO_GET_SERIALIZED_DATA,
            FAILED_TO_CREATE_COMPOSED_PROGRAM
        };

    public:
        // this will use 'PATH.getShadersPath().generic_string().c_str()' if you leave the argument 'search_paths' as default
        SlangParser(std::span<const char*> full_search_paths = {});
        ~SlangParser() = default;

        FORR_CLASS_MOVABLE(SlangParser)
        FORR_CLASS_NONCOPYABLE(SlangParser)

        // TODO : firstly pass whole 'fe::PipelineDesc', then collapse this function, making the class more modular
        static FORR_NODISCARD std::expected<shader::ProgramSources, ShaderBuildErrors> BuildShaderSources(const graphics::PipelineDesc& pipeline_desc, const ResourceManager& resource_manager);

        // TODO : return whole 'resource::ShaderFileData', then collapse this function, making the class more modular
        static FORR_NODISCARD std::expected<resource::ShaderFileData, ShaderFileDataErrors> BuildShaderFileData(const std::filesystem::path& resource_full_path, const ResourceStorage& storage);

    private:
        static FORR_NODISCARD Slang::ComPtr<slang::IComponentType> specialize(const graphics::PipelineDesc&        pipeline_desc,
                                                                              slang::IComponentType*               composed_program,
                                                                              std::vector<slang::IComponentType*>& component_types);

        // this reflects descriptors and push constants
        static void parseVariableRecursive(slang::VariableLayoutReflection*             variable_layout,
                                           std::vector<shader::ReflectedDescriptor>&    descriptor_layouts,
                                           std::vector<shader::ReflectedPushConstants>& push_constants_layouts);

        static void parseDescriptorTable(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDescriptor& dst_descriptor);
        static void parsePushConstants(slang::VariableLayoutReflection* variable_layout, shader::ReflectedPushConstants& dst_push_constants);

        static void parseMemberRecursive(slang::VariableLayoutReflection* variable_layout, shader::ReflectedDataNode* dst_reflected_data_node);
        static void parseMemberRecursive(slang::TypeLayoutReflection* type_layout, shader::ReflectedDataNode* dst_reflected_data_node);

        static void mapMatrix(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        static void mapVector(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);
        static void mapScalar(slang::TypeLayoutReflection* type_layout, shader::ValueType& type);

    private:
        inline static Slang::ComPtr<slang::IGlobalSession> m_GlobalSession{};
        inline static Slang::ComPtr<slang::ISession>       m_Session{};

        inline static std::string m_UnknownVariableName{ "[UNKNOWN]" };
    };
} // namespace fe
