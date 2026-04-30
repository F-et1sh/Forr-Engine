/*===============================================

    Forr Engine

    File : OpenGLResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "OpenGLResourceManager.hpp"

using namespace fe::resource;

template <>
void fe::OpenGLResourceManager::CreateResource(Material& material) {
    OpenGLMaterial opengl_material{};

    auto vertex_shader   = m_ResourceManager.GetResource(material.vertex_shader_ptr);
    auto fragment_shader = m_ResourceManager.GetResource(material.fragment_shader_ptr);

    this->createShaderProgram(opengl_material, { vertex_shader, fragment_shader });

    this->storeResource(material.gpu_handle, opengl_material, m_StorageMaterials);
}
template void fe::OpenGLResourceManager::CreateResource(Material& material);

///

template <>
void fe::OpenGLResourceManager::CreateResource(Model& model) {
    for (auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }
}
template void fe::OpenGLResourceManager::CreateResource(Model& model);

///

template <>
void fe::OpenGLResourceManager::CreateResource(Texture& texture) {
    OpenGLTexture opengl_texture{};

    int min_filter{};
    int mag_filter{};

    int wrap_s{};
    int wrap_t{};

    GLenum internal_format{};
    GLenum data_format{};

    // clang-format off
    switch (texture.min_filter) {
        case Texture::MinFilter::NEAREST               : min_filter = GL_NEAREST               ; break;
        case Texture::MinFilter::LINEAR                : min_filter = GL_LINEAR                ; break;
        case Texture::MinFilter::NEAREST_MIPMAP_NEAREST: min_filter = GL_NEAREST_MIPMAP_NEAREST; break;
        case Texture::MinFilter::LINEAR_MIPMAP_NEAREST : min_filter = GL_LINEAR_MIPMAP_NEAREST ; break;
        case Texture::MinFilter::NEAREST_MIPMAP_LINEAR : min_filter = GL_NEAREST_MIPMAP_LINEAR ; break;
        case Texture::MinFilter::LINEAR_MIPMAP_LINEAR  : min_filter = GL_LINEAR_MIPMAP_LINEAR  ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported min filter %i. Using GL_LINEAR as default", texture.min_filter);
            min_filter = GL_LINEAR;
    }

    // clang-format off
    switch (texture.mag_filter) {
        case Texture::MagFilter::NEAREST: mag_filter = GL_NEAREST; break;
        case Texture::MagFilter::LINEAR : mag_filter = GL_LINEAR ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported mag filter %i. Using GL_LINEAR as default", texture.mag_filter);
            mag_filter = GL_LINEAR;
    }
    // clang-format on

    // clang-format off
    switch (texture.wrap_s) {
        case Texture::Wrap::CLAMP_TO_EDGE  : wrap_s = GL_CLAMP_TO_EDGE  ; break;
        case Texture::Wrap::MIRRORED_REPEAT: wrap_s = GL_MIRRORED_REPEAT; break;
        case Texture::Wrap::REPEAT         : wrap_s = GL_REPEAT         ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported wrap s %i. Using GL_REPEAT as default", texture.wrap_s);
            wrap_s = GL_REPEAT;
    }
    // clang-format on

    // clang-format off
    switch (texture.wrap_t) {
        case Texture::Wrap::CLAMP_TO_EDGE  : wrap_t = GL_CLAMP_TO_EDGE  ; break;
        case Texture::Wrap::MIRRORED_REPEAT: wrap_t = GL_MIRRORED_REPEAT; break;
        case Texture::Wrap::REPEAT         : wrap_t = GL_REPEAT         ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported wrap t %i. Using GL_REPEAT as default", texture.wrap_t);
            wrap_t = GL_REPEAT;
    }
    // clang-format on

    // clang-format off
    switch (texture.internal_format) {
        case Texture::InternalFormat::RGBA8       : internal_format = GL_RGBA8       ; break;
        case Texture::InternalFormat::RGB8        : internal_format = GL_RGB8        ; break;
        case Texture::InternalFormat::RG8         : internal_format = GL_RG8         ; break;
        case Texture::InternalFormat::R8          : internal_format = GL_R8          ; break;
        case Texture::InternalFormat::SRGB8_ALPHA8: internal_format = GL_SRGB8_ALPHA8; break;
        case Texture::InternalFormat::SRGB8       : internal_format = GL_SRGB8       ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported internal format %i. Using GL_RGBA8 as default", texture.internal_format);
            internal_format = GL_RGBA8;
    }
    // clang-format on

    // clang-format off
    switch (texture.data_format) {
        case Texture::DataFormat::RGBA: data_format = GL_RGBA; break;
        case Texture::DataFormat::RGB : data_format = GL_RGB ; break;
        case Texture::DataFormat::RG  : data_format = GL_RG  ; break;
        case Texture::DataFormat::RED : data_format = GL_RED ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported data format %i. Using GL_RGBA as default", texture.data_format);
            data_format = GL_RGBA;
    }
    // clang-format on

    glCreateTextures(GL_TEXTURE_2D, 1, &opengl_texture.id);
    glBindTexture(GL_TEXTURE_2D, opengl_texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture.width, texture.height, 0, data_format, GL_UNSIGNED_BYTE, texture.bytes.get());

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    this->storeResource(texture.gpu_handle, opengl_texture, m_StorageTextures);
}
template void fe::OpenGLResourceManager::CreateResource(Texture& texture);

///

template<>
const fe::OpenGLMaterial& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Material> handle) const {
    return m_StorageMaterials[handle.index];
}
template const fe::OpenGLMaterial& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Material> handle)const;

template<>
const fe::OpenGLShaderProgram& fe::OpenGLResourceManager::GetResource(GPUHandle<OpenGLShaderProgram> handle) const {
    return m_StorageShaderPrograms[handle.index];
}
template const fe::OpenGLShaderProgram& fe::OpenGLResourceManager::GetResource(GPUHandle<OpenGLShaderProgram> handle)const;

template<>
const fe::OpenGLMesh& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Model::Mesh> handle) const {
    return m_StorageMeshes[handle.index];
}
template const fe::OpenGLMesh& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Model::Mesh> handle)const;


///

fe::GPUHandle<Model::Mesh> fe::OpenGLResourceManager::createMesh(resource::Model::Mesh& mesh) {
    OpenGLMesh opengl_mesh{};

    GLuint vao{};
    GLuint vbo{};
    GLuint ebo{};

    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    constexpr GLsizei stride = sizeof(Vertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glCreateBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    opengl_mesh.primitives.reserve(mesh.primitives.size());

    for (const auto& primitive : mesh.primitives) {
        auto& opengl_primitive = opengl_mesh.primitives.emplace_back();

        opengl_primitive.index_count  = primitive.index_count;
        opengl_primitive.index_offset = primitive.index_offset;

        // clang-format off
        switch (primitive.render_mode) {
            case RenderMode::POINTS        : opengl_primitive .render_mode = GL_POINTS        ; break;
            case RenderMode::LINES         : opengl_primitive .render_mode = GL_LINES         ; break;
            case RenderMode::LINE_LOOP     : opengl_primitive .render_mode = GL_LINE_LOOP     ; break;
            case RenderMode::LINE_STRIP    : opengl_primitive .render_mode = GL_LINE_STRIP    ; break;
            case RenderMode::TRIANGLES     : opengl_primitive .render_mode = GL_TRIANGLES     ; break;
            case RenderMode::TRIANGLE_STRIP: opengl_primitive .render_mode = GL_TRIANGLE_STRIP; break;
            case RenderMode::TRIANGLE_FAN  : opengl_primitive .render_mode = GL_TRIANGLE_FAN  ; break;
            default:
                fe::logging::warning("Unified -> OpenGL. Unsupported render mode %i. Using GL_TRIANGLES as default", primitive.render_mode);
                opengl_primitive .render_mode = GL_TRIANGLES;
        }
        // clang-format on
    }

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    opengl_mesh.vao.attach(vao);
    opengl_mesh.vbo.attach(vbo);
    opengl_mesh.ebo.attach(ebo);

    return GPUHandle<Model::Mesh>(this->storeResource(mesh.gpu_handle, opengl_mesh, m_StorageMeshes));
}

fe::GPUHandle<fe::OpenGLShaderProgram> fe::OpenGLResourceManager::createShaderProgram(OpenGLMaterial& opengl_material, std::vector<resource::Shader*> shaders) {
    OpenGLShaderProgram opengl_shader_program_raii{};
    GLuint              opengl_shader_program = glCreateProgram();

    for (size_t i = 0; i < shaders.size(); i++) {
        const auto& shader = shaders[i];

        unsigned int opengl_type{};
        unsigned int opengl_shader{};

        // clang-format off
        switch (shader->type) {
            case resource::Shader::Type::VERTEX: opengl_type = GL_VERTEX_SHADER; break;
            case resource::Shader::Type::FRAGMENT: opengl_type = GL_FRAGMENT_SHADER; break;
        }
        // clang-format on

        opengl_shader = glCreateShader(opengl_type);

        glShaderBinary(1, &opengl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, shader->source_code.data(), shader->source_code.size() * sizeof(uint32_t));
        glSpecializeShader(opengl_shader, "main", 0, nullptr, nullptr);

        glCompileShader(opengl_shader);

        int result = 0;
        glGetShaderiv(opengl_shader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            int length = 0;
            glGetShaderiv(opengl_shader, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*) _malloca(length * sizeof(char));
            glGetShaderInfoLog(opengl_shader, length, &length, message);

            fe::logging::error("Unified -> OpenGL. Failed to compile a shader\nMessage : %s", message);
        }
        else {
            glAttachShader(opengl_shader_program, opengl_shader);
        }

        glDeleteShader(opengl_shader);
    }

    glLinkProgram(opengl_shader_program);
    glValidateProgram(opengl_shader_program);

    opengl_shader_program_raii.shader_program.attach(opengl_shader_program);

    return GPUHandle<OpenGLShaderProgram>(this->storeResource(opengl_material.shader_program_handle, opengl_shader_program_raii, m_StorageShaderPrograms));
}