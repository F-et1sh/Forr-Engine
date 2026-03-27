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
#define FORR_RESOURCE_BODY(T) \
    FORR_CLASS_NONCOPYABLE(T) \
    FORR_CLASS_MOVABLE(T)

    struct OpenGLTexture {
        GLuint id{};

        OpenGLTexture()  = default;
        ~OpenGLTexture() = default;

        FORR_RESOURCE_BODY(OpenGLTexture)
    };

    struct OpenGLPrimitive {
        GLenum render_mode{};

        uint32_t index_offset{};
        uint32_t index_count{};

        fe::pointer<resource::Material> material{};

        OpenGLPrimitive()  = default;
        ~OpenGLPrimitive() = default;

        FORR_RESOURCE_BODY(OpenGLPrimitive)
    };

    struct OpenGLMesh {
        GLuint vao{};
        GLuint vbo{};
        GLuint ebo{};

        std::vector<OpenGLPrimitive> primitives{};

        OpenGLMesh()  = default;
        ~OpenGLMesh() = default;

        FORR_RESOURCE_BODY(OpenGLMesh)
    };

    struct OpenGLModel {
        std::vector<fe::pointer<OpenGLMesh>>    pointers_mesh{};
        std::vector<fe::pointer<OpenGLTexture>> pointers_texture{};

        OpenGLModel()  = default;
        ~OpenGLModel() = default;

        FORR_RESOURCE_BODY(OpenGLModel)
    };

#undef FORR_RESOURCE_BODY
} // namespace fe
