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

    struct OpenGLTexture { // TODO : provide textures
        GLuint id{};

        OpenGLTexture()  = default;
        ~OpenGLTexture() = default;

        FORR_RESOURCE_BODY(OpenGLTexture)
    };

    struct OpenGLShaderProgram {
        fe::gl::ShaderProgram shader_program{};

        OpenGLShaderProgram()  = default;
        ~OpenGLShaderProgram() = default;

        FORR_RESOURCE_BODY(OpenGLShaderProgram)
    };

    struct OpenGLMaterial {
        fe::pointer<OpenGLShaderProgram> shader_program_ptr{};

        OpenGLMaterial()  = default;
        ~OpenGLMaterial() = default;

        FORR_RESOURCE_BODY(OpenGLMaterial)
    };

    struct OpenGLPrimitive {
        GLenum render_mode{};

        uint32_t index_offset{};
        uint32_t index_count{};

        fe::pointer<resource::Material> material_ptr{};

        OpenGLPrimitive()  = default;
        ~OpenGLPrimitive() = default;

        FORR_RESOURCE_BODY(OpenGLPrimitive)
    };

    struct OpenGLMesh {
        fe::gl::VertexArray vao{};
        fe::gl::Buffer      vbo{};
        fe::gl::Buffer      ebo{};

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

    template <typename T>
    concept opengl_resource_t =
        (std::is_same_v<T, OpenGLTexture>) ||
        (std::is_same_v<T, OpenGLMesh>) ||
        (std::is_same_v<T, OpenGLShaderProgram>) ||
        (std::is_same_v<T, OpenGLMaterial>) ||
        (std::is_same_v<T, OpenGLModel>);

#undef FORR_RESOURCE_BODY
} // namespace fe
