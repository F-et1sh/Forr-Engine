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
        GPUHandle<OpenGLShaderProgram> shader_program_handle{};
        // std::vector<uint8_t> buffer{}; - take this from the CPU material ( fe::resource::Material )

        OpenGLMaterial()  = default;
        ~OpenGLMaterial() = default;

        FORR_RESOURCE_BODY(OpenGLMaterial)
    };

    struct OpenGLPrimitive {
        GLenum render_mode{};

        uint32_t index_offset{};
        uint32_t index_count{};

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

    template <typename T>
    concept opengl_resource_t =
        (std::is_same_v<T, OpenGLTexture>) ||
        (std::is_same_v<T, OpenGLMesh>) ||
        (std::is_same_v<T, OpenGLShaderProgram>) ||
        (std::is_same_v<T, OpenGLMaterial>);

    template <typename T>
    struct OpenGLResourceTraits;

#define OPENGL_RESOURCE_TRAITS_INSTANCE(CPU_TYPE, GPU_TYPE) \
    template <>                                             \
    struct OpenGLResourceTraits<CPU_TYPE> {                 \
        using type = GPU_TYPE;                              \
    };

    OPENGL_RESOURCE_TRAITS_INSTANCE(resource::Texture, OpenGLTexture)
    OPENGL_RESOURCE_TRAITS_INSTANCE(resource::Model::Mesh, OpenGLMesh)
    //OPENGL_RESOURCE_TRAITS_INSTANCE(resource::Shader, OpenGLShaderProgram) // this mustn't work because 'resource::Shader' is a single shader and 'OpenGLShaderProgram' is a program, which contains at least 2 shaders
    OPENGL_RESOURCE_TRAITS_INSTANCE(OpenGLShaderProgram, OpenGLShaderProgram) // this needs to use 'GPUHandle<OpenGLShaderProgram>' in GPU side
    OPENGL_RESOURCE_TRAITS_INSTANCE(resource::Material, OpenGLMaterial)

#undef FORR_RESOURCE_BODY
} // namespace fe
