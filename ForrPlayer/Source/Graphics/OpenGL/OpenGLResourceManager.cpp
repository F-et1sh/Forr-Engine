/*===============================================

    Forr Engine

    File : OpenGLResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "OpenGLResourceManager.hpp"

fe::MeshID fe::OpenGLResourceManager::CreateTriangle() {
    std::filesystem::path path = PATH.getModelsPath() / "StatueOfLiberty" / "statue_of_liberty.glb";
    /*std::vector<Vertex>   triangle_vertices{
        Vertex{ glm::vec3{ -0.5f, -0.5f, 0.0f } },
        Vertex{ glm::vec3{ 0.0f, 0.5f, 0.0f } },
        Vertex{ glm::vec3{ 0.5f, -0.5f, 0.0f } }
    };

    std::vector<uint8_t> triangle_indices{
        0, 1, 2
    };

    auto& mesh = m_Meshes.emplace_back(triangle_vertices, triangle_indices);

    mesh.vao.bind();

    mesh.vbo.bind();

    constexpr GLsizei stride = sizeof(Vertex);

    mesh.vao.LinkAttrib(mesh.vbo, 0, 3, GL_FLOAT, stride, (void*) offsetof(Vertex, position));

    mesh.ebo.bind();

    mesh.vbo.unbind();
    mesh.vao.unbind();
    mesh.ebo.unbind();

    return m_Meshes.size() - 1;*/

    return 0;
}

void fe::OpenGLResourceManager::CreateTexture(const resource::Texture& texture) {
    OpenGLTexture opengl_texture{};

    int min_filter{};
    int mag_filter{};

    int wrap_s{};
    int wrap_t{};

    GLenum internal_format{};
    GLenum data_format{};

    switch (texture.min_filter) {
        case TextureMinFilter::NEAREST:
            min_filter = GL_NEAREST;
            break;
        case TextureMinFilter::LINEAR:
            min_filter = GL_LINEAR;
            break;
        case TextureMinFilter::NEAREST_MIPMAP_NEAREST:
            min_filter = GL_NEAREST_MIPMAP_NEAREST;
            break;
        case TextureMinFilter::LINEAR_MIPMAP_NEAREST:
            min_filter = GL_LINEAR_MIPMAP_NEAREST;
            break;
        case TextureMinFilter::NEAREST_MIPMAP_LINEAR:
            min_filter = GL_NEAREST_MIPMAP_LINEAR;
            break;
        case TextureMinFilter::LINEAR_MIPMAP_LINEAR:
            min_filter = GL_LINEAR_MIPMAP_LINEAR;
            break;
        default:
            fe::logging::warning("Unknown min filter %i", texture.min_filter);
            break;
    }

    switch (texture.mag_filter) {
        case TextureMagFilter::NEAREST:
            mag_filter = GL_NEAREST;
            break;
        case TextureMagFilter::LINEAR:
            mag_filter = GL_LINEAR;
            break;
        default:
            fe::logging::warning("Unknown mag filter %i", texture.mag_filter);
            break;
    }

    switch (texture.wrap_s) {
        case TextureWrap::CLAMP_TO_EDGE:
            wrap_s = GL_CLAMP_TO_EDGE;
            break;
        case TextureWrap::MIRRORED_REPEAT:
            wrap_s = GL_MIRRORED_REPEAT;
            break;
        case TextureWrap::REPEAT:
            wrap_s = GL_REPEAT;
            break;
        default:
            fe::logging::warning("Unknown wrap s %i", texture.wrap_s);
            break;
    }

    switch (texture.wrap_t) {
        case TextureWrap::CLAMP_TO_EDGE:
            wrap_t = GL_CLAMP_TO_EDGE;
            break;
        case TextureWrap::MIRRORED_REPEAT:
            wrap_t = GL_MIRRORED_REPEAT;
            break;
        case TextureWrap::REPEAT:
            wrap_t = GL_REPEAT;
            break;
        default:
            fe::logging::warning("Unknown wrap t %i", texture.wrap_t);
            break;
    }

    switch (texture.internal_format) {
        case TextureInternalFormat::RGBA8:
            internal_format = GL_RGBA8;
            break;
        case TextureInternalFormat::RGB8:
            internal_format = GL_RGB8;
            break;
        case TextureInternalFormat::RG8:
            internal_format = GL_RG8;
            break;
        case TextureInternalFormat::R8:
            internal_format = GL_R8;
            break;
        case TextureInternalFormat::SRGB8_ALPHA8:
            internal_format = GL_SRGB8_ALPHA8;
            break;
        case TextureInternalFormat::SRGB8:
            internal_format = GL_SRGB8;
            break;
        default:
            fe::logging::warning("Unknown internal format %i", texture.internal_format);
            break;
    }

    switch (texture.data_format) {
        case TextureDataFormat::RGBA:
            data_format = GL_RGBA;
            break;
        case TextureDataFormat::RGB:
            data_format = GL_RGB;
            break;
        case TextureDataFormat::RG:
            data_format = GL_RG;
            break;
        case TextureDataFormat::RED:
            data_format = GL_RED;
            break;
        default:
            fe::logging::warning("Unknown data format %i", texture.data_format);
            break;
    }

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
    for (const auto& primitive : mesh.primitives) {
        this->createPrimitive(primitive);
    }
}

void fe::OpenGLResourceManager::createPrimitive(const Primitive& primitive) {
    OpenGLPrimitive opengl_primitive{};
    
    
}
