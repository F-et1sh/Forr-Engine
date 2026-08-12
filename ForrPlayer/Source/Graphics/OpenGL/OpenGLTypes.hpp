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
#include "OpenGLRAII.hpp"

namespace fe {
#define FORR_RESOURCE_BODY(T) \
    FORR_CLASS_NONCOPYABLE(T) \
    FORR_CLASS_MOVABLE(T)

    struct OpenGLTexture {
        fe::gl::Texture texture{};
        GLuint64        resident_id{};

        OpenGLTexture() = default;

        FORR_RESOURCE_BODY(OpenGLTexture)
    };

    struct OpenGLShaderBuffer {
        struct Binding {
            uint8_t*       mapped{};
            fe::gl::Buffer buffer{};
            size_t         size{};

            Binding()  = default;
            ~Binding() = default;

            FORR_RESOURCE_BODY(Binding)
        };

        std::vector<Binding> bindings{};

        OpenGLShaderBuffer() = default;

        FORR_RESOURCE_BODY(OpenGLShaderBuffer)
    };

    struct OpenGLPipeline {
        fe::gl::ShaderProgram shader_program{};

        bool   depth_test_enable{ true };
        GLenum depth_mode{ GL_LESS };

        bool   cull_enable{ true };
        GLenum cull_mode{ GL_FRONT };

        OpenGLPipeline() = default;

        FORR_RESOURCE_BODY(OpenGLPipeline)
    };

    struct OpenGLPrimitive {
        GLenum render_mode{};

        uint32_t index_offset{};
        uint32_t index_count{};

        OpenGLPrimitive() = default;

        FORR_RESOURCE_BODY(OpenGLPrimitive)
    };

    struct OpenGLMesh {
        fe::gl::VertexArray vao{};
        fe::gl::Buffer      vbo{};
        fe::gl::Buffer      ebo{};

        std::vector<OpenGLPrimitive> primitives{};

        OpenGLMesh() = default;

        FORR_RESOURCE_BODY(OpenGLMesh)
    };

#undef FORR_RESOURCE_BODY
} // namespace fe
