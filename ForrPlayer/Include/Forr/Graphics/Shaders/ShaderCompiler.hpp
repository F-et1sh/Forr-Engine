/*===============================================

    Forr Engine

    File : ShaderCompiler.hpp
    Role : shader compiler

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IRenderer.hpp"
#include "ResourceManagement/Resources.hpp"

namespace fe {
    class ShaderCompiler {
    public:
        ShaderCompiler()  = default;
        ~ShaderCompiler() = default;

        static void Compile(std::vector<uint32_t>& dst, std::string_view src, resource::Shader::Type shader_type, GraphicsBackend graphics_backend);

    private:
    };
} // namespace fe
