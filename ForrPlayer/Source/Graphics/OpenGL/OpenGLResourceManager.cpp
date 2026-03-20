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

fe::pointer<fe::OpenGLTexture> fe::OpenGLResourceManager::CreateTexture(fe::pointer<fe::resource::Texture> cpu_texture_ptr) {
    const auto& texture = *m_ResourceManager.GetResource(cpu_texture_ptr);

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

    auto gpu_ptr = m_Textures.create(opengl_texture);
    m_CPU_GPU_Texture.insert({ cpu_texture_ptr.packed(), gpu_ptr.packed() });

    return gpu_ptr;
}

void fe::OpenGLResourceManager::CreateModel(fe::pointer<fe::resource::Model> cpu_model_ptr) { // TODO : the texture or model might be already created, handle it
    OpenGLModel opengl_model{};

    // TODO : review all of this

    //const auto& model = *m_ResourceManager.GetResource(cpu_model_ptr);

    //// mesh hasn't its own extension, so you get a structure here, not a pointer
    //for (const auto& mesh : model.meshes) {
    //    auto ptr = this->createMesh(mesh);
    //    opengl_model.pointers_mesh.emplace_back(ptr);

    //    for (const auto& primitive : mesh.primitives) {
    //        auto material    = m_ResourceManager.GetResource(primitive.material_ptr);

    //        if (!material) continue; // TODO : provide fallbacks

    //        auto texture_ptr = material->pbr_metallic_roughness.base_color_texture.texture_ptr;

    //        auto ptr = this->CreateTexture(texture_ptr);
    //        opengl_model.pointers_texture.emplace_back(ptr);

    //        // TODO : change this to this :
    //        // auto& texture = *m_ResourceManager.GetResource(texture_ptr);
    //        // auto ptr = this->createTexture(texture);
    //        // opengl_model.pointers_texture.emplace_back(ptr);
    //    }
    //}

    auto gpu_ptr = m_Models.create(std::move(opengl_model));
    m_CPU_GPU_Model.insert({ cpu_model_ptr.packed(), gpu_ptr.packed() });
}

//fe::pointer<fe::OpenGLMesh> fe::OpenGLResourceManager::createMesh(const Mesh& mesh) {
//    OpenGLMesh opengl_mesh{};
//
//    for (const auto& primitive : mesh.primitives) {
//
//        OpenGLPrimitive opengl_primitive{};
//
//        this->createPrimitive(primitive, opengl_primitive);
//
//        opengl_mesh.primitives.emplace_back(std::move(opengl_primitive));
//    }
//
//    auto ptr = m_Meshes.create(std::move(opengl_mesh));
//    return ptr;
//}
//
//void fe::OpenGLResourceManager::createPrimitive(const Primitive& primitive, OpenGLPrimitive& opengl_primitive) {
//    //opengl_primitive.material // TODO : handle materials
//
//    glCreateVertexArrays(1, &opengl_primitive.vao);
//    glBindVertexArray(opengl_primitive.vao);
//
//    glCreateBuffers(1, &opengl_primitive.vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, opengl_primitive.vbo);
//
//    constexpr GLsizei stride = sizeof(Vertex);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, position));
//    glEnableVertexAttribArray(0);
//
//    // temp
//    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, index));
//    glEnableVertexAttribArray(1);
//
//    glCreateBuffers(1, &opengl_primitive.ebo);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opengl_primitive.ebo);
//
//    opengl_primitive.index_count  = primitive.index_count;
//    opengl_primitive.index_offset = primitive.index_offset;
//
//    // clang-format off
//    switch (primitive.render_mode) {
//        case RenderMode::POINTS        : opengl_primitive.render_mode = GL_POINTS        ; break;
//        case RenderMode::LINES         : opengl_primitive.render_mode = GL_LINES         ; break;
//        case RenderMode::LINE_LOOP     : opengl_primitive.render_mode = GL_LINE_LOOP     ; break;
//        case RenderMode::LINE_STRIP    : opengl_primitive.render_mode = GL_LINE_STRIP    ; break;
//        case RenderMode::TRIANGLES     : opengl_primitive.render_mode = GL_TRIANGLES     ; break;
//        case RenderMode::TRIANGLE_STRIP: opengl_primitive.render_mode = GL_TRIANGLE_STRIP; break;
//        case RenderMode::TRIANGLE_FAN  : opengl_primitive.render_mode = GL_TRIANGLE_FAN  ; break;
//        default:
//            fe::logging::warning("Unified -> OpenGL. Unsupported render mode %i. Using GL_TRIANGLES as default", primitive.render_mode);
//            opengl_primitive.render_mode = GL_TRIANGLES;
//    }
//    // clang-format on
//
//    glBufferData(GL_ARRAY_BUFFER, primitive.vertices.size() * sizeof(Vertex), primitive.vertices.data(), GL_STATIC_DRAW);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitive.indices.size() * sizeof(GLuint), primitive.indices.data(), GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//}
