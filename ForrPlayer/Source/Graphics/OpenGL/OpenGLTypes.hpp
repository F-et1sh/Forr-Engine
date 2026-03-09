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
#include "Core/pointer.hpp"
#include "ResourceManagement/Resources.hpp"
#include <glad/gl.h>

namespace fe {
    struct OpenGLTexture {
        GLuint id{};

        OpenGLTexture()  = default;
        ~OpenGLTexture() = default;
    };

    struct OpenGLPrimitive {
        uint32_t index_offset{};
        uint32_t index_count{};

        GLenum render_mode{ GL_TRIANGLES }; // triangles by default

        fe::pointer<resource::Material> material{};

        OpenGLPrimitive()  = default;
        ~OpenGLPrimitive() = default;
    };

    struct OpenGLMesh {
        GLuint vao{};
        GLuint vbo{};
        GLuint ebo{};

        // GLenum index_type{}; uint32_t for all models ( at least for now )
         
        std::vector<OpenGLPrimitive> primitives{};

        OpenGLMesh()  = default;
        ~OpenGLMesh() = default;
    };
} // namespace fe
