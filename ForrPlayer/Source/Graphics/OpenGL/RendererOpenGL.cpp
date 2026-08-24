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
      m_ResourceManager(resource_manager),
      m_OpenGLResourceManager(resource_manager) {

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

    { // temp
        m_Camera.setType(Camera::Type::LOOKAT);
        m_Camera.setPosition(glm::vec3(0.0f, 0.0f, -4.5f));
        m_Camera.setRotation(glm::vec3(0.0f));
        m_Camera.setFlipY(false);

        float speed  = 0.15f;
        float fov    = 60.0f;
        float aspect = (float) m_PrimaryWindow.getWidth() / (float) m_PrimaryWindow.getHeight();
        float znear  = 1.0f;
        float zfar   = 1000.0f;
        m_Camera.setPerspective(fov, aspect, znear, zfar);
        m_Camera.setMovementSpeed(speed);

        constexpr size_t     buffer_size = 16;
        constexpr GLbitfield flags       = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

        for (auto& push_constant : m_FramePushConstants) {
            auto& descriptor = push_constant.descriptor;

            GLuint buffer_raw{};
            glCreateBuffers(1, &buffer_raw);

            glNamedBufferStorage(buffer_raw, buffer_size, nullptr, flags);
            descriptor.mapped = static_cast<std::byte*>(glMapNamedBufferRange(buffer_raw, 0, buffer_size, flags));

            if (!descriptor.mapped) {
                glDeleteBuffers(1, &buffer_raw);
                fe::logging::error("OpenGL. Failed a uniform buffer for push constants. Mapped memory is nullptr");
                return;
            }

            descriptor.buffer.attach(buffer_raw);
            descriptor.size = buffer_size;
            descriptor.type = shader::DescriptorType::UNIFORM_BUFFER;
        }
    }
}

fe::RendererOpenGL::~RendererOpenGL() {
    glFinish();
}

fe::RenderGraphBindings fe::RendererOpenGL::CreateGPUResources(const RenderGraphCompileResult& compile_result) {
    RenderGraphBindings bindings{};
    bindings.image_bindings.reserve(compile_result.image_descs.size());

    for (const render_graph::ImageDesc& image_desc : compile_result.image_descs) {
        bindings.image_bindings[image_desc.handle.hashed_name] = m_OpenGLResourceManager.CreateImage(image_desc);
    }

    // TODO : provide buffers

    return bindings;
}

fe::ParameterID fe::RendererOpenGL::CreateParameter(const shader::ReflectedDescriptor& descriptor_layout) {
    return m_OpenGLResourceManager.CreateDescriptorRing(descriptor_layout);
}

void fe::RendererOpenGL::BindBuffer(ParameterID parameter_id) {
    OpenGLShaderDescriptorRing& descriptor_ring = m_OpenGLResourceManager.GetDescriptorRing(parameter_id.storage_index);
    OpenGLShaderDescriptor&     descriptor      = descriptor_ring[m_CurrentFrame];

    if (descriptor.type == shader::DescriptorType::STORAGE_BUFFER) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, static_cast<GLuint>(parameter_id.binding), descriptor.buffer.get());
    }
    else if (descriptor.type == shader::DescriptorType::UNIFORM_BUFFER) {
        glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(parameter_id.binding), descriptor.buffer.get());
    }
    else {
        fe::logging::error("OpenGL::BindBuffer() : Failed to bind buffer. Unsupported descriptor type %i", descriptor.type);
    }
}

void fe::RendererOpenGL::WriteBuffer(ParameterID parameter_id, std::span<const std::byte> data) {
    OpenGLShaderDescriptorRing& descriptor_ring = m_OpenGLResourceManager.GetDescriptorRing(parameter_id.storage_index);
    OpenGLShaderDescriptor&     descriptor      = descriptor_ring[m_CurrentFrame];
    std::memcpy(descriptor.mapped, data.data(), data.size());
}

void fe::RendererOpenGL::BeginFrame() {
    if (m_FrameData[m_CurrentFrame].sync) {
        glClientWaitSync(m_FrameData[m_CurrentFrame].sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    }
    
    m_FramePushConstants[m_CurrentFrame].current_offset = 0;
}

void fe::RendererOpenGL::EndFrame(const render_graph::CommandList& render_command_list) {
    render_command_list.handle_all([&](const auto& command) { this->handleCommand(command); });

    m_FrameData[m_CurrentFrame].sync.reset();
    GLsync sync_raw = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    m_FrameData[m_CurrentFrame].sync.attach(sync_raw);

    glfwSwapBuffers(m_GLFWwindow);

    // reset
    m_CurrentMaterial = {};
    m_CurrentMesh     = {};

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_CONCURRENT_FRAMES;
}

void fe::RendererOpenGL::InitializeGPUResources() {
    m_ResourceManager.RunForEach<resource::Texture>([&](resource::Texture& texture) {
        m_OpenGLResourceManager.CreateResource(texture);

        fe::logging::info("Loaded texture's size : %i %i", texture.width, texture.height);
    });

    m_ResourceManager.RunForEach<resource::Model>([&](resource::Model& model) {
        m_OpenGLResourceManager.CreateResource(model);

        fe::logging::info("Loaded model's mesh count %i", model.meshes.size());
    });
}

void fe::RendererOpenGL::bindPipeline(const OpenGLPipeline& pipeline) {
    glUseProgram(pipeline.shader_program.get());

    if (pipeline.depth_test_enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(pipeline.depth_mode);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    if (pipeline.cull_enable) { // TODO : enable this
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
    }
    else {
        glDisable(GL_CULL_FACE);
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
                                                  reinterpret_cast<void*>(static_cast<uintptr_t>(command.first_index * sizeof(uint32_t))),
                                                  command.instance_count,
                                                  command.vertex_offset,
                                                  command.first_instance);
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindShaderProgram& command) {
    m_BoundShaderProgramPtr = command.shader_program_ptr;
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindMaterial& command) {
    const OpenGLPipeline& pipeline = m_OpenGLResourceManager.GetOrCreatePipeline(m_BoundShaderProgramPtr, command.material_ptr);
    bindPipeline(pipeline);
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindModel& command) {
    auto&    push_constants = m_FramePushConstants[m_CurrentFrame];
    uint32_t current_offset = push_constants.current_offset;

    std::memcpy(push_constants.descriptor.mapped + current_offset, command.push_constants_data.data(), command.push_constants_data.size());

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, push_constants.descriptor.buffer.get(), current_offset, command.push_constants_data.size());

    static GLint alignment{};
    if (alignment == 0)
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

    push_constants.current_offset = (current_offset + command.push_constants_data.size() + alignment - 1) & ~(alignment - 1);

    const resource::Model& model = *m_ResourceManager.GetResource(command.model_ptr);

    for (const auto& mesh : model.meshes) {
        const auto& opengl_mesh = m_OpenGLResourceManager.GetResource(mesh.gpu_handle);
        glBindVertexArray(opengl_mesh.vao);

        for (const auto& primitive : opengl_mesh.primitives) {
            glDrawElements(primitive.render_mode, primitive.index_count, GL_UNSIGNED_INT, (void*) primitive.index_offset);
        }
    }
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindBuffer& command) {
    this->BindBuffer(command.parameter_id);
}

void fe::RendererOpenGL::handleCommand(const render_graph::WriteBuffer& command) {
    this->WriteBuffer(command.parameter_id, command.data);
}
