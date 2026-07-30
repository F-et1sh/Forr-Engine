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

void fe::OpenGLResourceManager::CreateResource(Material& material) {
    OpenGLMaterial opengl_material{};

    //ShaderProgram* shader_program = m_ResourceManager.GetResource(material.shader_program_ptr);
    //if (!shader_program) {
    //    fe::logging::error("Unified -> OpenGL. Failed to create material. material.shader_program.ptr was invalid");
    //    return;
    //}

    //OpenGLShaderProgram opengl_shader_program{};
    //GLuint              opengl_shader_program_raw = this->createShaderProgramRaw(*shader_program);
    //opengl_shader_program.shader_program.attach(opengl_shader_program_raw);

    //opengl_shader_program.shader_buffers.

    //for (const auto& sampler : material.samplers) {
    //    auto& texture = *m_ResourceManager.GetResource(sampler.texture_ptr);
    //    if (!texture.gpu_handle) {
    //        this->CreateResource(texture);
    //    }

    //    const auto& opengl_texture = this->GetResource(texture.gpu_handle);

    //    if (!material.buffer.empty())
    //        std::memcpy(&material.buffer[sampler.offset], &opengl_texture.resident_id, sizeof(uint64_t));
    //}

    this->storeResource(material.gpu_handle, opengl_material, m_StorageMaterials);
}

void fe::OpenGLResourceManager::CreateResource(Model& model) {
    for (auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }

    //m_StorageMaterials
    //std::
}

void fe::OpenGLResourceManager::CreateResource(Texture& texture) {
    OpenGLTexture opengl_texture{};

    GLuint texture_id_raw{};

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

    glCreateTextures(GL_TEXTURE_2D, 1, &texture_id_raw);
    glBindTexture(GL_TEXTURE_2D, texture_id_raw);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture.width, texture.height, 0, data_format, GL_UNSIGNED_BYTE, texture.bytes.get());

    // TODO : get mipmaps from 'texture.mip_levels'
    glGenerateMipmap(GL_TEXTURE_2D);

    opengl_texture.resident_id = glGetTextureHandleARB(texture_id_raw);
    glMakeTextureHandleResidentARB(opengl_texture.resident_id); // make resident

    glBindTexture(GL_TEXTURE_2D, 0);

    opengl_texture.texture.attach(texture_id_raw);

    this->storeResource(texture.gpu_handle, opengl_texture, m_StorageTextures);
}

size_t fe::OpenGLResourceManager::CreateImage(const render_graph::ImageDesc& image_desc) {
    auto& this_texture = m_StorageTextures.emplace_back();
    // TODO : create OpenGL texture - this_texture.texture
    return m_StorageTextures.size() - 1;
}

// TODO : provide fallbacks
const fe::OpenGLTexture& fe::OpenGLResourceManager::GetImage(size_t texture_storage_index) {
    if (m_StorageTextures.size() <= texture_storage_index) {
        fe::logging::fatal("Out of range");
    }
    return m_StorageTextures[texture_storage_index];
}

// TODO : this about this again | 29.07.2026 what did I mean ?
GLuint fe::OpenGLResourceManager::GetShaderBuffer(shader::ReflectedDescriptor& parameter) {
    auto it = m_ShaderBuffers.find(parameter);
    if (it != m_ShaderBuffers.end()) return it->second.get();

    using shader_descriptor = shader::DescriptorType;

    size_t buffer_size = 16 * 1024; // 16KB

    if (parameter.array_size != 0) {
        buffer_size = parameter.array_size * parameter.size;
    }

    GLuint buffer_raw{};
    glCreateBuffers(1, &buffer_raw);

    if (parameter.descriptor_type == shader_descriptor::UNIFORM_BUFFER) {
        glNamedBufferData(buffer_raw, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    }
    else if (parameter.descriptor_type == shader_descriptor::STORAGE_BUFFER) {
        GLbitfield flags = GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT |
                           GL_MAP_COHERENT_BIT;

        glNamedBufferStorage(buffer_raw, buffer_size, nullptr, flags);
    }
    else {
        glDeleteBuffers(1, &buffer_raw);
        fe::logging::warning("Unified -> OpenGL. Failed to create a buffer ( SSBO or UBO ) : unsupported descriptor type %i", parameter.descriptor_type);
        return ~0;
    }

    m_ShaderBuffers[parameter].attach(buffer_raw);
    return buffer_raw;
}

// TODO : provide fallbacks
#define GET_RESOURCE_INSTANCE(RETURN_T, HANDLE_T, STORAGE)                                     \
    const RETURN_T& fe::OpenGLResourceManager::GetResource(GPUHandle<HANDLE_T> handle) const { \
        if (STORAGE.size() <= handle.index) {                                                  \
            fe::logging::fatal("Out of range");                                                \
        }                                                                                      \
        return STORAGE[handle.index];                                                          \
    }

GET_RESOURCE_INSTANCE(fe::OpenGLMaterial, fe::resource::Material, m_StorageMaterials)
GET_RESOURCE_INSTANCE(fe::OpenGLMesh, fe::resource::Model::Mesh, m_StorageMeshes)
GET_RESOURCE_INSTANCE(fe::OpenGLTexture, fe::resource::Texture, m_StorageTextures)

#undef GET_RESOURCE_INSTANCE

fe::GPUHandle<fe::resource::Model::Mesh> fe::OpenGLResourceManager::createMesh(resource::Model::Mesh& mesh) {
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

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, texture_coord));
    glEnableVertexAttribArray(2);

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

GLuint fe::OpenGLResourceManager::createShaderProgramRaw(resource::ShaderProgram& shader_program) {
    GLuint opengl_shader_program_raw = glCreateProgram();

    for (const auto& [shader_type, source_code] : shader_program.source_codes) {
        unsigned int opengl_type{};
        unsigned int opengl_shader{};

        // clang-format off
        switch (shader_type) {
            case shader::StageBits::VERTEX  : opengl_type = GL_VERTEX_SHADER  ; break;
            case shader::StageBits::FRAGMENT: opengl_type = GL_FRAGMENT_SHADER; break;
        }
        // clang-format on

        opengl_shader = glCreateShader(opengl_type);

        glShaderBinary(1, &opengl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, source_code.data(), source_code.size());
        glSpecializeShader(opengl_shader, "main", 0, nullptr, nullptr);

        //const char* glsl_text_ptr = reinterpret_cast<const char*>(source_code.data());
        //GLint       length        = static_cast<GLint>(source_code.size());
        //glShaderSource(opengl_shader, 1, &glsl_text_ptr, &length);

        //glCompileShader(opengl_shader);

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
            glAttachShader(opengl_shader_program_raw, opengl_shader);
        }

        glDeleteShader(opengl_shader);
    };

    glLinkProgram(opengl_shader_program_raw);
    glValidateProgram(opengl_shader_program_raw);

    return opengl_shader_program_raw;
}
