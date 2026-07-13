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
        SlangParser();
        ~SlangParser() = default;

        FORR_CLASS_MOVABLE(SlangParser)
        FORR_CLASS_NONCOPYABLE(SlangParser)

        // primary processing : load .slang file, save its reflected and serialized data, for the secondary processing
        bool LoadFromFileAndReflect(const std::filesystem::path& resource_full_path, resource::ShaderReflectedData& shader_reflected_data);

    private:
        std::vector<EntryPoint> findEntryPoints(std::vector<slang::IComponentType*>& component_types);
        bool                    reflect(resource::ShaderReflectedData& shader_reflected_data);

        bool reflectMaterial(shader::ReflectedMaterialLayout& material_layout);
        bool reflectPipeline();

    private:
        Slang::ComPtr<slang::ISession> m_Session{};
        slang::IModule*                m_Module{};
        // this will be 'm_Module's layout by default, but if you compose a program - it will be layout of that program
        slang::ProgramLayout*          m_RootLayout{};
    };
} // namespace fe
