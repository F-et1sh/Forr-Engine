/*===============================================

    Forr Engine

    File : ShaderOpenGL.hpp
    Role : OpenGL Shader

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IShader.hpp"

namespace fe {
    class ShaderOpenGL : public IShader {
    public:
        ShaderOpenGL()  = default;
        ~ShaderOpenGL() = default;

        void Initialize(ShaderDesc& desc);

        void Bind();
        void Unbind();

    private:
    };
} // namespace fe
