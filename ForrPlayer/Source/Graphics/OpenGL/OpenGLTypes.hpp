/*===============================================

    Forr Engine

    File : OpenGLTypes.hpp
    Role : OpenGL types like Texture.
        There mustn't be complex thing. Just thin POD structures.
        This engine is mostly data-oriented designed

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <glad/gl.h>

namespace fe {
    struct OpenGLTexture {
        GLuint id{};

        OpenGLTexture()  = default;
        ~OpenGLTexture() = default;
    };

    struct OpenGLPrimitive {
        GLuint vertex_id{};
        GLuint index_id{};

        GLenum index_type{};

        OpenGLPrimitive()  = default;
        ~OpenGLPrimitive() = default;
    };
} // namespace fe
