/*===============================================

    Forr Engine

    File : RendererOpenGL.cpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "RendererOpenGL.hpp"

fe::RendererOpenGL::RendererOpenGL(const RendererDesc& desc,
                                   IPlatformSystem&    platform_system,
                                   size_t              primary_window_index,
                                   ResourceManager&    resource_manager)
    : m_PlatformSystem(platform_system),
      m_PrimaryWindow(m_PlatformSystem.getWindow(primary_window_index)),
      m_ResourceManager(resource_manager) {

    m_GLFWwindow = (GLFWwindow*) m_PrimaryWindow.getNativeHandle();

    glfwMakeContextCurrent(m_GLFWwindow);

    // TODO : move all of this chekings to other layer
    int is_bindless_supported = glfwExtensionSupported("GL_ARB_bindless_texture");
    if (!is_bindless_supported) {
        fe::logging::fatal("Your version of OpenGL does not support bindless rendering. Please, use Vulkan as the graphics backend");
        return;
    }

    glfwSwapInterval(desc.primary_window_desc.vsync); // set vsync ( only after calling glfwMakeContextCurrent )

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fe::logging::error("Failed to initialize OpenGL context");
        return;
    }
}

fe::RendererOpenGL::~RendererOpenGL() {
    glFinish();
}

fe::RenderGraphBindings fe::RendererOpenGL::CreateGPUResources(const RenderGraphCompileResult& compile_result) {
    RenderGraphBindings bindings{};
    //bindings.image_bindings.reserve(compile_result.image_descs.size());

    //for (const render_graph::ImageDesc& image_desc : compile_result.image_descs) {
    //    bindings.image_bindings[image_desc.handle.hashed_name] = m_OpenGLResourceManager.CreateImage(image_desc);
    //}

    //// TODO : provide buffers

    return bindings;
}

std::expected<fe::ParameterID, fe::ParameterCreationErrors> fe::RendererOpenGL::CreateParameter(const ParameterDesc& parameter_desc) {
    size_t buffer_size = 16 * 1024; // 16KB

    if (parameter_desc.array_size != 0) {
        buffer_size = parameter_desc.array_size * parameter_desc.size;
    }

    OpenGLShaderParameterRing descriptor_ring{};

    for (auto& descriptor : descriptor_ring) {
        GLuint buffer_raw{};
        glCreateBuffers(1, &buffer_raw);

        GLbitfield flags = GL_MAP_WRITE_BIT |
                           GL_MAP_PERSISTENT_BIT |
                           GL_MAP_COHERENT_BIT;

        if (parameter_desc.descriptor_type == shader::DescriptorType::UNIFORM_BUFFER) {
            glNamedBufferStorage(buffer_raw, buffer_size, nullptr, flags);
            descriptor.mapped = static_cast<std::byte*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));
        }
        else if (parameter_desc.descriptor_type == shader::DescriptorType::STORAGE_BUFFER) {
            glNamedBufferStorage(buffer_raw, buffer_size, nullptr, flags);
            descriptor.mapped = static_cast<std::byte*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));
        }
        else if (parameter_desc.descriptor_type == shader::DescriptorType::GENERIC) {
            return std::unexpected{ ParameterCreationErrors::FORGOT_TO_SPECIALIZE_GENERIC_DESCRIPTOR };
        }
        else {
            glDeleteBuffers(1, &buffer_raw);
            return std::unexpected{ ParameterCreationErrors::UNSUPPORTED_MEMORY_TYPE };
        }

        if (!descriptor.mapped) {
            glDeleteBuffers(1, &buffer_raw);
            return std::unexpected{ ParameterCreationErrors::MAPPED_MEMORY_WAS_NULLPTR };
        }

        descriptor.buffer.attach(buffer_raw);
        descriptor.size = buffer_size;
        descriptor.type = parameter_desc.descriptor_type;
    }

    return m_Parameters.emplace(std::move(descriptor_ring));
}

void fe::RendererOpenGL::BindParameter(ParameterID parameter_id) {
    OpenGLShaderParameterRing* descriptor_ring = m_Parameters.get(parameter_id);
    if (!descriptor_ring) {
        fe::logging::error("Failed to write parameter. Failed to get descriptor ring.\nParameterID :\nindex = %i\ngeneration = %i\nset = %i\nbinding = %i",
                           static_cast<uint32_t>(parameter_id.index()),
                           static_cast<uint32_t>(parameter_id.generation()),
                           static_cast<uint32_t>(parameter_id.custom_fields().set),
                           static_cast<uint32_t>(parameter_id.custom_fields().binding));
        return;
    }
    OpenGLShaderParameter& descriptor = descriptor_ring->operator[](m_CurrentFrame);

    if (descriptor.type == shader::DescriptorType::STORAGE_BUFFER) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(parameter_id.custom_fields().binding), descriptor.buffer.get());
    }
    else if (descriptor.type == shader::DescriptorType::UNIFORM_BUFFER) {
        glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(parameter_id.custom_fields().binding), descriptor.buffer.get());
    }
    else {
        fe::logging::error("OpenGL::BindBuffer() : Failed to bind buffer. Unsupported descriptor type %i", descriptor.type);
    }
}

void fe::RendererOpenGL::WriteParameter(ParameterID parameter_id, std::span<const std::byte> data) {
    OpenGLShaderParameterRing* descriptor_ring = m_Parameters.get(parameter_id);
    if (!descriptor_ring) {
        fe::logging::error("Failed to write parameter. Failed to get descriptor ring.\nParameterID :\nindex = %i\ngeneration = %i\nset = %i\nbinding = %i",
                           static_cast<uint32_t>(parameter_id.index()),
                           static_cast<uint32_t>(parameter_id.generation()),
                           static_cast<uint32_t>(parameter_id.custom_fields().set),
                           static_cast<uint32_t>(parameter_id.custom_fields().binding));
        return;
    }
    OpenGLShaderParameter& descriptor = descriptor_ring->operator[](m_CurrentFrame);
    std::memcpy(descriptor.mapped, data.data(), data.size());
}

void fe::RendererOpenGL::DestroyParameter(ParameterID parameter_id) {
    OpenGLShaderParameterRing* descriptor_ring = m_Parameters.get(parameter_id);
    if (!descriptor_ring) {
        fe::logging::error("Failed to destroy parameter. Failed to get descriptor ring.\nParameterID :\nindex = %i\ngeneration = %i\nset = %i\nbinding = %i",
                           static_cast<uint32_t>(parameter_id.index()),
                           static_cast<uint32_t>(parameter_id.generation()),
                           static_cast<uint32_t>(parameter_id.custom_fields().set),
                           static_cast<uint32_t>(parameter_id.custom_fields().binding));
        return;
    }

    auto& buffers_to_destory = m_FrameData[m_CurrentFrame].buffers_to_destroy;
    buffers_to_destory.reserve(buffers_to_destory.size() + MAX_CONCURRENT_FRAMES);

    for (auto& descriptor : *descriptor_ring) {
        buffers_to_destory.emplace_back(std::move(descriptor));
    }

    m_Parameters.destroy(parameter_id);
}

FORR_NODISCARD std::expected<fe::PipelineID, fe::PipelineCreationErrors> fe::RendererOpenGL::CreatePipeline(const PipelineDesc& pipeline_desc) {
    SlangParser slang_parser{};
    auto        source_codes = slang_parser.BuildShaderSources(pipeline_desc, m_ResourceManager);

    if (!source_codes.has_value()) {
        switch (source_codes.error()) {
            // TODO : provide correct errors here
            case fe::SlangParser::ShaderBuildErrors::ERROR:
                return std::unexpected{ PipelineCreationErrors::ERROR };
                break;
            default:
                return std::unexpected{ PipelineCreationErrors::ERROR };
                break;
        }
    }

    OpenGLPipeline opengl_pipeline{};

    GLuint shader_program_raw = this->createShaderProgramRaw(source_codes.value());
    opengl_pipeline.shader_program.attach(shader_program_raw);

    // clang-format off
    switch (pipeline_desc.pipeline_flags.render_mode) {
        case RenderMode::POINTS        : opengl_pipeline.render_mode = GL_POINTS        ; break;
        case RenderMode::LINES         : opengl_pipeline.render_mode = GL_LINES         ; break;
        case RenderMode::LINE_LOOP     : opengl_pipeline.render_mode = GL_LINE_LOOP     ; break;
        case RenderMode::LINE_STRIP    : opengl_pipeline.render_mode = GL_LINE_STRIP    ; break;
        case RenderMode::TRIANGLES     : opengl_pipeline.render_mode = GL_TRIANGLES     ; break;
        case RenderMode::TRIANGLE_STRIP: opengl_pipeline.render_mode = GL_TRIANGLE_STRIP; break;
        case RenderMode::TRIANGLE_FAN  : opengl_pipeline.render_mode = GL_TRIANGLE_FAN  ; break;
        default:
            fe::logging::warning("Unified -> OpenGL. Unsupported render mode %i. Using GL_TRIANGLES as default",
                pipeline_desc.pipeline_flags.render_mode);
            opengl_pipeline.render_mode = GL_TRIANGLES;
    }
    // clang-format on

    opengl_pipeline.depth_test_enable = pipeline_desc.pipeline_flags.depth_test_enable;
    // clang-format off
    switch (pipeline_desc.pipeline_flags.depth_mode) {
        case DepthMode::NEVER   : opengl_pipeline.depth_mode = GL_NEVER   ; break;
        case DepthMode::LESS    : opengl_pipeline.depth_mode = GL_LESS    ; break;
        case DepthMode::EQUAL   : opengl_pipeline.depth_mode = GL_EQUAL   ; break;
        case DepthMode::LEQUAL  : opengl_pipeline.depth_mode = GL_LEQUAL  ; break;
        case DepthMode::GREATER : opengl_pipeline.depth_mode = GL_GREATER ; break;
        case DepthMode::NOTEQUAL: opengl_pipeline.depth_mode = GL_NOTEQUAL; break;
        case DepthMode::GEQUAL  : opengl_pipeline.depth_mode = GL_GEQUAL  ; break;
        case DepthMode::ALWAYS  : opengl_pipeline.depth_mode = GL_ALWAYS  ; break;
            default:
                fe::logging::warning("Unified -> OpenGL. Unsupported depth mode %i. Using GL_LESS as default",
                    pipeline_desc.pipeline_flags.depth_mode);
            opengl_pipeline.depth_mode = GL_LESS;
    }
    // clang-format on

    opengl_pipeline.cull_enable = pipeline_desc.pipeline_flags.cull_enable;
    // clang-format off
    switch (pipeline_desc.pipeline_flags.cull_mode) {
        case CullMode::NONE          : opengl_pipeline.cull_mode = GL_NONE          ; break;
        case CullMode::FRONT         : opengl_pipeline.cull_mode = GL_FRONT         ; break;
        case CullMode::BACK          : opengl_pipeline.cull_mode = GL_BACK          ; break;
        case CullMode::FRONT_AND_BACK: opengl_pipeline.cull_mode = GL_FRONT_AND_BACK; break;
            default:
                fe::logging::warning("Unified -> OpenGL. Unsupported cull mode %i. Using GL_NONE as default",
                    pipeline_desc.pipeline_flags.cull_mode);
            opengl_pipeline.cull_mode = GL_NONE;
    }
    // clang-format on

    return m_Pipelines.emplace(std::move(opengl_pipeline));
}

void fe::RendererOpenGL::BindPipeline(PipelineID pipeline_id) {
    OpenGLPipeline* pipeline = m_Pipelines.get(pipeline_id);
    if (!pipeline) {
        fe::logging::error("Failed to bind pipeline. Failed to get pipeline.\nPipelineID :\nindex = %i\ngeneration = %i",
                           static_cast<uint32_t>(pipeline_id.index()),
                           static_cast<uint32_t>(pipeline_id.generation()));
        return;
    }

    glUseProgram(pipeline->shader_program.get());

    if (pipeline->depth_test_enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(pipeline->depth_mode);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    // TODO : enable this
    //
    ////if (pipeline.cull_enable) {
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);
    ////}
    ////else {
    ////glDisable(GL_CULL_FACE);
    ////}
}

void fe::RendererOpenGL::DestroyPipeline(PipelineID pipeline_id) {
    OpenGLPipeline* pipeline = m_Pipelines.get(pipeline_id);
    if (!pipeline) {
        fe::logging::error("Failed to destroy pipeline. Failed to get pipeline.\nPipelineID :\nindex = %i\ngeneration = %i",
                           static_cast<uint32_t>(pipeline_id.index()),
                           static_cast<uint32_t>(pipeline_id.generation()));
        return;
    }

    m_FrameData[m_CurrentFrame].shader_programs_to_destory.emplace_back(std::move(pipeline->shader_program));

    m_Pipelines.destroy(pipeline_id);
}

void fe::RendererOpenGL::BeginFrame() {
    if (m_FrameData[m_CurrentFrame].sync) {
        glClientWaitSync(m_FrameData[m_CurrentFrame].sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    }

    m_FrameData[m_CurrentFrame].buffers_to_destroy.clear();
    m_FrameData[m_CurrentFrame].shader_programs_to_destory.clear();
}

void fe::RendererOpenGL::EndFrame(const render_graph::CommandList& render_command_list) {
    render_command_list.handle_all([&](const auto& command) { this->handleCommand(command); });

    glfwSwapBuffers(m_GLFWwindow);

    m_FrameData[m_CurrentFrame].sync.reset();

    GLsync sync_raw = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    m_FrameData[m_CurrentFrame].sync.attach(sync_raw);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_CONCURRENT_FRAMES;
}

void fe::RendererOpenGL::InitializeGPUResources() {
    //m_ResourceManager.RunForEach<resource::Texture>([&](resource::Texture& texture) {
    //    m_OpenGLResourceManager.CreateResource(texture);

    //    fe::logging::info("Loaded texture's size : %i %i", texture.width, texture.height);
    //});

    //m_ResourceManager.RunForEach<resource::Model>([&](resource::Model& model) {
    //    m_OpenGLResourceManager.CreateResource(model);

    //    fe::logging::info("Loaded model's mesh count %i", model.meshes.size());
    //});
}

void fe::RendererOpenGL::bindPipeline(const OpenGLPipeline& pipeline) {
    //glUseProgram(pipeline.shader_program.get());

    //m_CurrentRenderMode = pipeline.render_mode;

    //if (pipeline.depth_test_enable) {
    //    glEnable(GL_DEPTH_TEST);
    //    glDepthFunc(pipeline.depth_mode);
    //}
    //else {
    //    glDisable(GL_DEPTH_TEST);
    //}

    //// TODO : enable this
    ////
    ////if (pipeline.cull_enable) {
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);
    ////}
    ////else {
    ////glDisable(GL_CULL_FACE);
    ////}
}

GLuint fe::RendererOpenGL::createShaderProgramRaw(const shader::ProgramSources& program_sources) {
    GLuint opengl_shader_program_raw = glCreateProgram();
    bool   compilation_failed{};

    for (const auto& [shader_type, source_code] : program_sources) {
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

void fe::RendererOpenGL::handleCommand(const render_graph::ImageBarrier& command) {
    const auto& opengl_texture = m_OpenGLResourceManager.GetImage(command.handle.storage_index);
    uint64_t    resident_id    = opengl_texture.resident_id;

    if (command.new_state == ResourceState::SHADER_READ_ONLY) {
        if (!glIsTextureHandleResidentARB(resident_id)) {
            glMakeTextureHandleResidentARB(resident_id);
        }

        if (command.old_state == ResourceState::UNORDERED_ACCESS) {
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }
    else if (command.new_state == ResourceState::RENDER_TARGET ||
             command.new_state == ResourceState::DEPTH_READ) {

        if (glIsTextureHandleResidentARB(resident_id)) {
            glMakeTextureHandleNonResidentARB(resident_id);
        }
    }
    else if (command.old_state == ResourceState::RENDER_TARGET &&
             command.new_state == ResourceState::UNORDERED_ACCESS) {

        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    }
}

void fe::RendererOpenGL::handleCommand(const render_graph::BufferBarrier& command) {
    // TODO : provide this
}

void fe::RendererOpenGL::handleCommand(const render_graph::BeginRenderPass& command) {
    if (command.is_to_screen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(command.viewport.offset.x,
                   command.viewport.offset.y,
                   command.viewport.extent.x,
                   command.viewport.extent.y);

        GLbitfield clear_mask{};
        if (command.is_clears_color) {
            glClearColor(command.clear_color_value.r,
                         command.clear_color_value.g,
                         command.clear_color_value.b,
                         command.clear_color_value.a);
            clear_mask |= GL_COLOR_BUFFER_BIT;
        }

        if (command.is_clears_depth) {
            glClearDepth(command.clear_depth_value);
            clear_mask |= GL_DEPTH_BUFFER_BIT;
        }

        if (clear_mask != 0) glClear(clear_mask);

        return;
    }

    GLuint framebuffer_raw{};

    uint64_t framebuffer_hash = render_graph::color_depth_targets_hash(command.color_targets,
                                                                       command.color_targets_count,
                                                                       command.depth_target);

    auto it = m_FramebuffersCache.find(framebuffer_hash);
    if (it != m_FramebuffersCache.end()) {
        framebuffer_raw = it->second.get();
    }
    else {
        glCreateFramebuffers(1, &framebuffer_raw);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_raw);

        for (size_t i = 0; i < command.color_targets_count; i++) {
            size_t               texture_index  = command.color_targets[i];
            const OpenGLTexture& opengl_texture = m_OpenGLResourceManager.GetImage(texture_index);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, opengl_texture.texture, 0);
        }

        if (command.has_depth_target) {
            const OpenGLTexture& opengl_texture = m_OpenGLResourceManager.GetImage(command.depth_target);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, opengl_texture.texture, 0);
        }

        std::vector<GLenum> attachments{};

        for (size_t i = 0; i < command.color_targets_count; i++) {
            attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
        }

        if (attachments.empty()) {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
        else {
            glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fe::logging::error("Unified RenderGraph -> OpenGL. Framebuffer status is incomplete");
        }

        m_FramebuffersCache[framebuffer_hash] = std::move(gl::Framebuffer{ framebuffer_raw });
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_raw);
    glViewport(0, 0, command.viewport.extent.x, command.viewport.extent.y);

    GLbitfield clear_mask{};
    if (command.is_clears_color) {
        glClearColor(command.clear_color_value.r,
                     command.clear_color_value.g,
                     command.clear_color_value.b,
                     command.clear_color_value.a);
        clear_mask |= GL_COLOR_BUFFER_BIT;
    }

    if (command.is_clears_depth) {
        glClearDepth(command.clear_depth_value);
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (clear_mask != 0) glClear(clear_mask);
}

void fe::RendererOpenGL::handleCommand(const render_graph::EndRenderPass& end_render_pass) {
    glBindVertexArray(0);
    glUseProgram(0);
}

void fe::RendererOpenGL::handleCommand(const render_graph::DrawIndexed& command) {
    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                  command.index_count,
                                                  GL_UNSIGNED_INT,
                                                  reinterpret_cast<void*>(command.first_index * sizeof(uint32_t)),
                                                  command.instance_count,
                                                  command.vertex_offset,
                                                  command.first_instance);
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindPipeline& command) {
    //m_BoundShaderProgramPtr = command.shader_program_ptr;
}

void fe::RendererOpenGL::handleCommand(const render_graph::DrawModel& command) {
    //const resource::Model& model = *m_ResourceManager.GetResource(command.model_ptr);

    //for (const auto& mesh : model.meshes) {
    //    const auto& opengl_mesh = m_OpenGLResourceManager.GetResource(mesh.gpu_handle);
    //    glBindVertexArray(opengl_mesh.vao);

    //    for (const auto& primitive : opengl_mesh.primitives) {
    //        glDrawElementsInstancedBaseVertexBaseInstance(m_CurrentRenderMode,
    //                                                      primitive.index_count,
    //                                                      GL_UNSIGNED_INT,
    //                                                      (void*) primitive.index_offset,
    //                                                      1,
    //                                                      0,
    //                                                      command.first_instance);
    //    }
    //}
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindBuffer& command) {
    //this->BindBuffer(command.parameter_id);
}

void fe::RendererOpenGL::handleCommand(const render_graph::WriteBuffer& command) {
    //this->WriteBuffer(command.parameter_id, command.data);
}
