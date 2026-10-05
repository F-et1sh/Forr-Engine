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
            ERROR,
            COMPOSITION_FAILED
        };

    public:
        // this will use 'PATH.getShadersPath().generic_string().c_str()' if you leave the argument 'search_paths' as default
        SlangParser(std::span<const char*> full_search_paths = {});
        ~SlangParser() = default;

        FORR_CLASS_MOVABLE(SlangParser)
        FORR_CLASS_NONCOPYABLE(SlangParser)

        // TODO : firstly pass whole 'fe::PipelineDesc', then collapse this function, making the class more modular
        std::expected<shader::ProgramSources, ShaderBuildErrors> BuildShaderSources(const graphics::PipelineDesc& pipeline_desc, ResourceManager& resource_manager);

    private:
        Slang::ComPtr<slang::ISession> m_Session{};
    };
} // namespace fe
