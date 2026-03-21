/*===============================================

    Forr Engine

    File : OpenGLTypes.hpp
    Role : OpenGL types. All structures here must be movable only
        Even if the structure is only 4 bytes

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

        FORR_CLASS_NONCOPYABLE(OpenGLTexture)
        FORR_CLASS_MOVABLE(OpenGLTexture)
    };

    struct OpenGLPrimitive {
        GLenum render_mode{};

        uint32_t index_offset{};
        uint32_t index_count{};

        fe::pointer<resource::Material> material{};

        OpenGLPrimitive()  = default;
        ~OpenGLPrimitive() = default;

        FORR_CLASS_NONCOPYABLE(OpenGLPrimitive)
        FORR_CLASS_MOVABLE(OpenGLPrimitive)
    };

    struct OpenGLMesh {
        GLuint vao{};
        GLuint vbo{};
        GLuint ebo{};

        std::vector<OpenGLPrimitive> primitives{};

        OpenGLMesh()  = default;
        ~OpenGLMesh() = default;

        FORR_CLASS_NONCOPYABLE(OpenGLMesh)
        FORR_CLASS_MOVABLE(OpenGLMesh)
    };

    struct OpenGLModel {
        std::vector<fe::pointer<OpenGLMesh>>    pointers_mesh{};
        std::vector<fe::pointer<OpenGLTexture>> pointers_texture{};

        OpenGLModel()  = default;
        ~OpenGLModel() = default;

        FORR_CLASS_NONCOPYABLE(OpenGLModel)
        FORR_CLASS_MOVABLE(OpenGLModel)
    };
} // namespace fe
