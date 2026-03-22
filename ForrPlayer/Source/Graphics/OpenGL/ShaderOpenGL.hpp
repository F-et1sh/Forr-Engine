/*===============================================

    Forr Engine

    File : ShaderOpenGL.hpp
    Role : OpenGL Shader

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IShader.hpp"

#include <glad/gl.h>

namespace fe {
    class ShaderOpenGL : public IShader {
    public:
        ShaderOpenGL()  = default;
        ~ShaderOpenGL() = default;

        void Initialize(ShaderDesc& desc);

        void Bind();
        void Unbind();

    private:
        void loadSource(const std::filesystem::path& full_path, const char* source_dst);
        void attachVertexShader(const std::filesystem::path& full_path);
        void attachFragmentShader(const std::filesystem::path& full_path);

    private:
        GLuint m_Program = 0;
    };
} // namespace fe
