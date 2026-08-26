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

    struct OpenGLShaderDescriptor {
        size_t                 size{};
        std::byte*             mapped{};
        shader::DescriptorType type{}; // UBO or SSBO
        fe::gl::Buffer         buffer{};

        OpenGLShaderDescriptor() = default;

        FORR_RESOURCE_BODY(OpenGLShaderDescriptor)
    };

    // due frame sync there must be 2 or 3 descriptors intead of one
    // this is like a default 'fe::OpenGLShaderDescriptor' but for per-frame using without artifacts
    using OpenGLShaderDescriptorRing = std::array<OpenGLShaderDescriptor, MAX_CONCURRENT_FRAMES>;

    struct OpenGLPipeline {
        fe::gl::ShaderProgram shader_program{};

        GLenum render_mode{};

        bool   depth_test_enable{ true };
        GLenum depth_mode{ GL_LESS };

        bool   cull_enable{ true };
        GLenum cull_mode{ GL_ALWAYS }; // doesn't work right now

        OpenGLPipeline() = default;

        FORR_RESOURCE_BODY(OpenGLPipeline)
    };

    struct OpenGLPrimitive {
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
