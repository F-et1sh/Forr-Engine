/*===============================================

    Forr Engine

    File : OpenGLResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "OpenGLResourceManager.hpp"

void fe::OpenGLResourceManager::CreateTexture(const resource::Texture& texture) {
    OpenGLTexture opengl_texture{};

    int min_filter{};
    int mag_filter{};

    int wrap_s{};
    int wrap_t{};

    GLenum internal_format{};
    GLenum data_format{};

    // clang-format off
    switch (texture.min_filter) {
        case TextureMinFilter::NEAREST: min_filter = GL_NEAREST; break;
        case TextureMinFilter::LINEAR: min_filter = GL_LINEAR; break;
        case TextureMinFilter::NEAREST_MIPMAP_NEAREST: min_filter = GL_NEAREST_MIPMAP_NEAREST; break;
        case TextureMinFilter::LINEAR_MIPMAP_NEAREST: min_filter = GL_LINEAR_MIPMAP_NEAREST; break;
        case TextureMinFilter::NEAREST_MIPMAP_LINEAR: min_filter = GL_NEAREST_MIPMAP_LINEAR; break;
        case TextureMinFilter::LINEAR_MIPMAP_LINEAR: min_filter = GL_LINEAR_MIPMAP_LINEAR; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported min filter %i", texture.min_filter);
            break;
    }

    switch (texture.mag_filter) {
        case TextureMagFilter::NEAREST: mag_filter = GL_NEAREST; break;
        case TextureMagFilter::LINEAR: mag_filter = GL_LINEAR; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported mag filter %i", texture.mag_filter);
            break;
    }

    switch (texture.wrap_s) {
        case TextureWrap::CLAMP_TO_EDGE: wrap_s = GL_CLAMP_TO_EDGE; break;
        case TextureWrap::MIRRORED_REPEAT: wrap_s = GL_MIRRORED_REPEAT; break;
        case TextureWrap::REPEAT: wrap_s = GL_REPEAT; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported wrap s %i", texture.wrap_s);
            break;
    }

    switch (texture.wrap_t) {
        case TextureWrap::CLAMP_TO_EDGE: wrap_t = GL_CLAMP_TO_EDGE; break;
        case TextureWrap::MIRRORED_REPEAT: wrap_t = GL_MIRRORED_REPEAT; break;
        case TextureWrap::REPEAT: wrap_t = GL_REPEAT; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported wrap t %i", texture.wrap_t);
            break;
    }

    switch (texture.internal_format) {
        case TextureInternalFormat::RGBA8: internal_format = GL_RGBA8; break;
        case TextureInternalFormat::RGB8: internal_format = GL_RGB8; break;
        case TextureInternalFormat::RG8: internal_format = GL_RG8; break;
        case TextureInternalFormat::R8: internal_format = GL_R8; break;
        case TextureInternalFormat::SRGB8_ALPHA8: internal_format = GL_SRGB8_ALPHA8; break;
        case TextureInternalFormat::SRGB8: internal_format = GL_SRGB8; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported internal format %i", texture.internal_format);
            break;
    }

    switch (texture.data_format) {
        case TextureDataFormat::RGBA: data_format = GL_RGBA; break;
        case TextureDataFormat::RGB: data_format = GL_RGB; break;
        case TextureDataFormat::RG: data_format = GL_RG; break;
        case TextureDataFormat::RED: data_format = GL_RED; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported data format %i", texture.data_format);
            break;
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

    auto ptr = m_Textures.create(opengl_texture); // does not need to store this pointer
}

void fe::OpenGLResourceManager::CreateModel(const resource::Model& model) {
    // mesh hasn't its own extension, so you get a structure here, not a pointer
    for (const auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }

    // texture has its own extension, so you get a pointer here
    for (const auto& texture_ptr : model.textures) {
        auto texture = m_ResourceManager.GetResource(texture_ptr);
        this->CreateTexture(*texture); // TODO : the texture might be already created
    }
}

void fe::OpenGLResourceManager::createMesh(const Mesh& mesh) {
    OpenGLMesh opengl_mesh{};

    Vertices vertices{};
    Indices  indices{};

    glCreateVertexArrays(1, &opengl_mesh.vao);
    glBindVertexArray(opengl_mesh.vao);

    glCreateBuffers(1, &opengl_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, opengl_mesh.vbo);

    constexpr GLsizei stride = sizeof(Vertex);

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_SHORT, stride, (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glCreateBuffers(1, &opengl_mesh.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl_mesh.ebo);

    for (const auto& primitive : mesh.primitives) {

        OpenGLPrimitive opengl_primitive{};

        this->createPrimitive(primitive, opengl_primitive, vertices, indices);
        opengl_mesh.primitives.emplace_back(opengl_primitive);
    }

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void fe::OpenGLResourceManager::createPrimitive(const Primitive& primitive, OpenGLPrimitive& opengl_primitive, Vertices& vertices, Indices& indices) {
    //opengl_primitive.material // TODO : handle materials

    opengl_primitive.index_offset = primitive.index_offset;
    opengl_primitive.index_count  = primitive.index_count;

    // clang-format off
    switch (primitive.render_mode) {
        case RenderMode::POINTS: opengl_primitive.render_mode = GL_POINTS; break;
        case RenderMode::LINES: opengl_primitive.render_mode = GL_LINES; break;
        case RenderMode::LINE_LOOP: opengl_primitive.render_mode = GL_LINE_LOOP; break;
        case RenderMode::LINE_STRIP: opengl_primitive.render_mode = GL_LINE_STRIP; break;
        case RenderMode::TRIANGLES: opengl_primitive.render_mode = GL_TRIANGLES; break;
        case RenderMode::TRIANGLE_STRIP: opengl_primitive.render_mode = GL_TRIANGLE_STRIP; break;
        case RenderMode::TRIANGLE_FAN: opengl_primitive.render_mode = GL_TRIANGLE_FAN; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported render mode %i. Using GL_TRIANGLES as default", primitive.render_mode);
            opengl_primitive.render_mode = GL_TRIANGLES;
            break;
    }
    // clang-format on

    vertices.assign_range(primitive.vertices);
    indices.assign_range(primitive.indices);
}
