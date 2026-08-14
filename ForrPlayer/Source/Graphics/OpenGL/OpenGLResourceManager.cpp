/*===============================================

    Forr Engine

    File : OpenGLResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "OpenGLResourceManager.hpp"

#include "Graphics/Slang/SlangParser.hpp"

using namespace fe::resource;

void fe::OpenGLResourceManager::CreateResource(Model& model) {
    for (auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }
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
            fe::logging::error("Unified -> OpenGL. Unsupported min filter %i. Using GL_LINEAR as default", texture.min_filter);
            min_filter = GL_LINEAR;
    }

    // clang-format off
    switch (texture.mag_filter) {
        case Texture::MagFilter::NEAREST: mag_filter = GL_NEAREST; break;
        case Texture::MagFilter::LINEAR : mag_filter = GL_LINEAR ; break;
        default:
            fe::logging::error("Unified -> OpenGL. Unsupported mag filter %i. Using GL_LINEAR as default", texture.mag_filter);
            mag_filter = GL_LINEAR;
    }
    // clang-format on

    // clang-format off
    switch (texture.wrap_s) {
        case Texture::Wrap::CLAMP_TO_EDGE  : wrap_s = GL_CLAMP_TO_EDGE  ; break;
        case Texture::Wrap::MIRRORED_REPEAT: wrap_s = GL_MIRRORED_REPEAT; break;
        case Texture::Wrap::REPEAT         : wrap_s = GL_REPEAT         ; break;
        default:
            fe::logging::error("Unified -> OpenGL. Unsupported wrap s %i. Using GL_REPEAT as default", texture.wrap_s);
            wrap_s = GL_REPEAT;
    }
    // clang-format on

    // clang-format off
    switch (texture.wrap_t) {
        case Texture::Wrap::CLAMP_TO_EDGE  : wrap_t = GL_CLAMP_TO_EDGE  ; break;
        case Texture::Wrap::MIRRORED_REPEAT: wrap_t = GL_MIRRORED_REPEAT; break;
        case Texture::Wrap::REPEAT         : wrap_t = GL_REPEAT         ; break;
        default:
            fe::logging::error("Unified -> OpenGL. Unsupported wrap t %i. Using GL_REPEAT as default", texture.wrap_t);
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
            fe::logging::error("Unified -> OpenGL. Unsupported internal format %i. Using GL_RGBA8 as default", texture.internal_format);
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
            fe::logging::error("Unified -> OpenGL. Unsupported data format %i. Using GL_RGBA as default", texture.data_format);
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

    GLuint opengl_texture_raw{};
    GLenum target{};

    switch (image_desc.type) {
        case render_graph::ImageType::IMAGE_TYPE_1D:
            target = GL_TEXTURE_1D;
            break;
        case render_graph::ImageType::IMAGE_TYPE_2D:
            target = GL_TEXTURE_2D;
            break;
        case render_graph::ImageType::IMAGE_TYPE_3D:
            target = GL_TEXTURE_3D;
            break;
        default:
            fe::logging::error("Unified RenderGraph -> OpenGL. Unsupported image type %i. Using GL_TEXTURE_2D as default", image_desc.type);
            target = GL_TEXTURE_2D;
    }

    glCreateTextures(target, 1, &opengl_texture_raw);
    glBindTexture(target, opengl_texture_raw);

    GLenum internal_format = GL_RGBA8;
    GLenum data_format     = GL_RGBA;
    GLenum data_type       = GL_UNSIGNED_BYTE;

    // clang-format off
    switch (image_desc.format) {
        case render_graph::Format::RGBA8_UNORM       : internal_format = GL_RGBA8                ; data_format = GL_RGBA             ; data_type = GL_UNSIGNED_BYTE                  ; break;
        case render_graph::Format::RGBA8_SRGB        : internal_format = GL_SRGB8_ALPHA8         ; data_format = GL_RGBA             ; data_type = GL_UNSIGNED_BYTE                  ; break;
        case render_graph::Format::BGRA8_UNORM       : internal_format = GL_RGBA8                ; data_format = GL_BGRA             ; data_type = GL_UNSIGNED_BYTE                  ; break;
        case render_graph::Format::RGBA16_SFLOAT     : internal_format = GL_RGBA16F              ; data_format = GL_RGBA             ; data_type = GL_FLOAT                          ; break;
        case render_graph::Format::R11G11B10_SFLOAT  : internal_format = GL_R11F_G11F_B10F       ; data_format = GL_RGB              ; data_type = GL_FLOAT                          ; break;
        case render_graph::Format::RG16_SFLOAT       : internal_format = GL_RG16F                ; data_format = GL_RG               ; data_type = GL_FLOAT                          ; break;
        case render_graph::Format::R32_UINT          : internal_format = GL_R32UI                ; data_format = GL_RED_INTEGER      ; data_type = GL_UNSIGNED_INT                   ; break;
        case render_graph::Format::R32_SFLOAT        : internal_format = GL_R32F                 ; data_format = GL_RED              ; data_type = GL_FLOAT                          ; break;
        case render_graph::Format::D32_SFLOAT        : internal_format = GL_DEPTH_COMPONENT32F   ; data_format = GL_DEPTH_COMPONENT  ; data_type = GL_FLOAT                          ; break;
        case render_graph::Format::D24_UNORM_S8_UINT : internal_format = GL_DEPTH24_STENCIL8     ; data_format = GL_DEPTH_STENCIL    ; data_type = GL_UNSIGNED_INT_24_8              ; break;
        case render_graph::Format::D32_SFLOAT_S8_UINT: internal_format = GL_DEPTH32F_STENCIL8    ; data_format = GL_DEPTH_STENCIL    ; data_type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV ; break;
        
        default:
            fe::logging::warning("Unified RenderGraph -> OpenGL. Unsupported format %i. Using GL_RGBA8 as default", image_desc.format);
    }
    // clang-format on

    // clang-format off
    switch (image_desc.type) {
        case render_graph::ImageType::IMAGE_TYPE_1D: glTexImage1D(GL_TEXTURE_1D, 0, internal_format, image_desc.extent.x                                          , 0, data_format, data_type, nullptr); break;
        case render_graph::ImageType::IMAGE_TYPE_2D: glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image_desc.extent.x, image_desc.extent.y                     , 0, data_format, data_type, nullptr); break;
        case render_graph::ImageType::IMAGE_TYPE_3D: glTexImage3D(GL_TEXTURE_3D, 0, internal_format, image_desc.extent.x, image_desc.extent.y, image_desc.extent.z, 0, data_format, data_type, nullptr); break;
    }
    // clang-format on

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(target, 0);

    this_texture.resident_id = glGetTextureHandleARB(opengl_texture_raw);
    this_texture.texture.attach(opengl_texture_raw);

    return m_StorageTextures.size() - 1;
}

// TODO : provide fallbacks
const fe::OpenGLTexture& fe::OpenGLResourceManager::GetImage(size_t texture_storage_index) {
    if (m_StorageTextures.size() <= texture_storage_index) {
        fe::logging::fatal("Out of range");
    }
    return m_StorageTextures[texture_storage_index];
}

const fe::OpenGLPipeline& fe::OpenGLResourceManager::GetOrCreatePipeline(fe::pointer<resource::ShaderProgram> shader_program_ptr,
                                                                         fe::pointer<resource::Material>      material_ptr) {
    size_t seed{};
    fe::hash_combine(seed, shader_program_ptr.packed());
    fe::hash_combine(seed, material_ptr.packed());

    auto it = m_Pipelines.find(seed);
    if (it != m_Pipelines.end()) return it->second;

    OpenGLPipeline& opengl_pipeline = m_Pipelines[seed];

    const resource::ShaderProgram& shader_program = *m_ResourceManager.GetResource(shader_program_ptr);
    const resource::Material&      material       = *m_ResourceManager.GetResource(material_ptr);

    SlangParser               parser{ m_ResourceManager.GetContext().graphics_backend };
    shader::SourceCodeStorage source_codes = parser.CombineAndCompileShader(shader_program, material, m_ResourceManager);

    if (source_codes.empty()) {
        if (!m_Pipelines.empty())
            return m_Pipelines[0]; // TODO : provide fallbacks
        fe::logging::fatal("Out of range");
    }

    GLuint shader_program_raw = this->createShaderProgramRaw(source_codes);
    opengl_pipeline.shader_program.attach(shader_program_raw);

    opengl_pipeline.depth_test_enable = material.pipeline_flags.depth_test_enable;
    // clang-format off
    switch (material.pipeline_flags.depth_mode) {
        case DepthMode::NEVER   : opengl_pipeline.depth_mode = GL_NEVER   ; break;
        case DepthMode::LESS    : opengl_pipeline.depth_mode = GL_LESS    ; break;
        case DepthMode::EQUAL   : opengl_pipeline.depth_mode = GL_EQUAL   ; break;
        case DepthMode::LEQUAL  : opengl_pipeline.depth_mode = GL_LEQUAL  ; break;
        case DepthMode::GREATER : opengl_pipeline.depth_mode = GL_GREATER ; break;
        case DepthMode::NOTEQUAL: opengl_pipeline.depth_mode = GL_NOTEQUAL; break;
        case DepthMode::GEQUAL  : opengl_pipeline.depth_mode = GL_GEQUAL  ; break;
        case DepthMode::ALWAYS  : opengl_pipeline.depth_mode = GL_ALWAYS  ; break;
    }
    // clang-format on

    opengl_pipeline.cull_enable = material.pipeline_flags.cull_enable;
    // clang-format off
    switch (material.pipeline_flags.cull_mode) {
        case CullMode::NONE          : opengl_pipeline.cull_mode = GL_NONE          ; break;
        case CullMode::FRONT         : opengl_pipeline.cull_mode = GL_FRONT         ; break;
        case CullMode::BACK          : opengl_pipeline.cull_mode = GL_BACK          ; break;
        case CullMode::FRONT_AND_BACK: opengl_pipeline.cull_mode = GL_FRONT_AND_BACK; break;
    }
    // clang-format on

    return opengl_pipeline;
}

const fe::OpenGLShaderDescriptorRing& fe::OpenGLResourceManager::GetOrCreateShaderBuffer(const shader::ReflectedDescriptor& parameter) {
    auto it = m_ShaderBuffers.find(parameter);
    if (it != m_ShaderBuffers.end()) return it->second;

    size_t buffer_size = 16 * 1024; // 16KB

    if (parameter.array_size != 0) {
        buffer_size = parameter.array_size * parameter.size;
    }

    auto& descriptor_ring = m_ShaderBuffers[parameter];

    for (auto& descriptor : descriptor_ring) {
        GLuint buffer_raw{};
        glCreateBuffers(1, &buffer_raw);

        GLbitfield flags = GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT |
                           GL_MAP_COHERENT_BIT;

        if (parameter.descriptor_type == shader::DescriptorType::UNIFORM_BUFFER) {
            glNamedBufferData(buffer_raw, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            descriptor.mapped = static_cast<uint8_t*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));
        }
        else if (parameter.descriptor_type == shader::DescriptorType::STORAGE_BUFFER) {
            glNamedBufferStorage(buffer_raw, buffer_size, nullptr, flags);
            descriptor.mapped = static_cast<uint8_t*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));
        }
        else {
            glDeleteBuffers(1, &buffer_raw);
            fe::logging::error("Unified -> OpenGL. Failed to create a buffer ( SSBO or UBO ) : unsupported descriptor type %i",
                               parameter.descriptor_type);
            return {};
        }

        descriptor.buffer.attach(buffer_raw);
        descriptor.size = buffer_size;
    }

    return descriptor_ring;
}

fe::ParameterID fe::OpenGLResourceManager::CreateParameter(const shader::ReflectedDescriptor& descriptor_layout) {
    ParameterID parameter_id{};
    parameter_id.set           = descriptor_layout.set;
    parameter_id.binding       = descriptor_layout.binding;
    parameter_id.storage_index = m_ShaderBuffers.size();

    size_t buffer_size = 16 * 1024; // 16KB

    if (descriptor_layout.array_size != 0) {
        buffer_size = descriptor_layout.array_size * descriptor_layout.size;
    }

    OpenGLShaderDescriptorRing descriptor_ring{};

    for (auto& descriptor : descriptor_ring) {
        GLuint buffer_raw{};
        glCreateBuffers(1, &buffer_raw);

        GLbitfield flags = GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT |
                           GL_MAP_COHERENT_BIT;

        if (descriptor_layout.descriptor_type == shader::DescriptorType::UNIFORM_BUFFER) {
            glNamedBufferData(buffer_raw, buffer_size, nullptr, GL_DYNAMIC_DRAW);
            descriptor.mapped = static_cast<uint8_t*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));
        }
        else if (descriptor_layout.descriptor_type == shader::DescriptorType::STORAGE_BUFFER) {
            glNamedBufferStorage(buffer_raw, buffer_size, nullptr, flags);
            descriptor.mapped = static_cast<uint8_t*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));
        }
        else {
            glDeleteBuffers(1, &buffer_raw);
            fe::logging::error("Unified -> OpenGL. Failed to create a buffer ( SSBO or UBO ) : unsupported descriptor type %i",
                               descriptor_layout.descriptor_type);
            return {};
        }

        descriptor.buffer.attach(buffer_raw);
        descriptor.size = buffer_size;
    }

    m_ShaderBuffers.emplace_back(std::move(descriptor_ring));

    return parameter_id;
}

// TODO : provide fallbacks
#define GET_RESOURCE_INSTANCE(RETURN_T, HANDLE_T, STORAGE)                                     \
    const RETURN_T& fe::OpenGLResourceManager::GetResource(GPUHandle<HANDLE_T> handle) const { \
        if (STORAGE.size() <= handle.index) {                                                  \
            fe::logging::fatal("Out of range");                                                \
        }                                                                                      \
        return STORAGE[handle.index];                                                          \
    }

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

GLuint fe::OpenGLResourceManager::createShaderProgramRaw(const fe::shader::SourceCodeStorage& source_codes) {
    GLuint opengl_shader_program_raw = glCreateProgram();
    bool   compilation_failed{};

    for (const auto& [shader_type, source_code] : source_codes) {
        unsigned int opengl_type{};
        unsigned int opengl_shader{};

        // clang-format off
        switch (shader_type) {
            case shader::StageBits::VERTEX  : opengl_type = GL_VERTEX_SHADER  ; break;
            case shader::StageBits::FRAGMENT: opengl_type = GL_FRAGMENT_SHADER; break;
            case shader::StageBits::GEOMETRY: opengl_type = GL_GEOMETRY_SHADER; break;
            case shader::StageBits::COMPUTE : opengl_type = GL_COMPUTE_SHADER ; break;
        }
        // clang-format on

        opengl_shader = glCreateShader(opengl_type);

        glShaderBinary(1, &opengl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, source_code.data(), source_code.size());
        glSpecializeShader(opengl_shader, "main", 0, nullptr, nullptr);

        // code for GLSL importing

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
            compilation_failed = true;
        }
        else {
            glAttachShader(opengl_shader_program_raw, opengl_shader);
        }

        glDeleteShader(opengl_shader);
    };

    if (compilation_failed) {
        glDeleteProgram(opengl_shader_program_raw);
        return 0;
    }
    else {
        glLinkProgram(opengl_shader_program_raw);
        glValidateProgram(opengl_shader_program_raw);

        return opengl_shader_program_raw;
    }
}
