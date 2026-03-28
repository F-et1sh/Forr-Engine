/*===============================================

    Forr Engine

    File : OpenGLResourceCreator.cpp
    Role : stores OpenGL resources

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "OpenGLResourceCreator.hpp"

using namespace fe::resource;

fe::pointer<fe::OpenGLTexture> fe::OpenGLResourceCreator::CreateResource(const resource::Texture& texture) {
    OpenGLTexture this_texture{};

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

    glCreateTextures(GL_TEXTURE_2D, 1, &this_texture.id);
    glBindTexture(GL_TEXTURE_2D, this_texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture.width, texture.height, 0, data_format, GL_UNSIGNED_BYTE, texture.bytes.get());

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    auto ptr = m_Storage.m_Textures.create(std::move(this_texture));
    return ptr;
}

FORR_NODISCARD fe::pointer<fe::OpenGLModel> fe::OpenGLResourceCreator::CreateResource(const resource::Model& model) {
    OpenGLModel this_model{};

    this_model.pointers_mesh.reserve(model.meshes.size());

    for (const auto& mesh : model.meshes) {
        auto ptr = this->createMesh(mesh);
        this_model.pointers_mesh.emplace_back(ptr);
    }

    auto ptr = m_Storage.m_Models.create(std::move(this_model));
    return ptr;
}

FORR_NODISCARD fe::pointer<fe::OpenGLShader> fe::OpenGLResourceCreator::CreateResource(const resource::Shader& shader) {
    OpenGLShader this_shader{};

    //this_shader.program_id
    //shader.source

    auto ptr = m_Storage.m_Shaders.create(std::move(this_shader));
    return ptr;
}

fe::pointer<fe::OpenGLMesh> fe::OpenGLResourceCreator::createMesh(const resource::Model::Mesh& mesh) {
    OpenGLMesh this_mesh{};

    glCreateVertexArrays(1, &this_mesh.vao);
    glBindVertexArray(this_mesh.vao);

    glCreateBuffers(1, &this_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this_mesh.vbo);

    constexpr GLsizei stride = sizeof(Vertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glCreateBuffers(1, &this_mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this_mesh.ebo);

    this_mesh.primitives.reserve(mesh.primitives.size());
    for (const auto& primitive : mesh.primitives) {
        auto& this_primitive = this_mesh.primitives.emplace_back();

        this_primitive.index_count  = primitive.index_count;
        this_primitive.index_offset = primitive.index_offset;

        // clang-format off
        switch (primitive.render_mode) {
            case RenderMode::POINTS        : this_primitive.render_mode = GL_POINTS        ; break;
            case RenderMode::LINES         : this_primitive.render_mode = GL_LINES         ; break;
            case RenderMode::LINE_LOOP     : this_primitive.render_mode = GL_LINE_LOOP     ; break;
            case RenderMode::LINE_STRIP    : this_primitive.render_mode = GL_LINE_STRIP    ; break;
            case RenderMode::TRIANGLES     : this_primitive.render_mode = GL_TRIANGLES     ; break;
            case RenderMode::TRIANGLE_STRIP: this_primitive.render_mode = GL_TRIANGLE_STRIP; break;
            case RenderMode::TRIANGLE_FAN  : this_primitive.render_mode = GL_TRIANGLE_FAN  ; break;
            default:
                fe::logging::warning("Unified -> OpenGL. Unsupported render mode %i. Using GL_TRIANGLES as default", primitive.render_mode);
                this_primitive.render_mode = GL_TRIANGLES;
        }
        // clang-format on
    }

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    auto ptr = m_Storage.m_Meshes.create(std::move(this_mesh));
    return ptr;
}
