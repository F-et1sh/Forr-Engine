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

    glfwSwapInterval(desc.primary_window_desc.vsync); // set vsync ( only after calling glfwMakeContextCurrent )

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fe::logging::error("Failed to initialize OpenGL context");
        return;
    }

    glViewport(0, 0, m_PrimaryWindow.getWidth(), m_PrimaryWindow.getHeight());
    glEnable(GL_DEPTH_TEST);

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
    }
}

fe::RendererOpenGL::~RendererOpenGL() {
    glFinish();
}

void fe::RendererOpenGL::SetClearColor(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
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

void fe::RendererOpenGL::BeginFrame() {
    if (m_FrameData[m_CurrentFrame].sync) {
        glClientWaitSync(m_FrameData[m_CurrentFrame].sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    }
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

    if (pipeline.cull_enable) {
        glEnable(GL_CULL_FACE);
        glCullFace(pipeline.cull_mode);
    }
    else {
        glDisable(GL_CULL_FACE);
    }
}

void fe::RendererOpenGL::handleCommand(const render_graph::ImageBarrier& image_barrier) {
    const auto& opengl_texture = m_OpenGLResourceManager.GetImage(image_barrier.handle.storage_index);
    uint64_t    resident_id    = opengl_texture.resident_id;

    if (image_barrier.new_state == ResourceState::SHADER_READ_ONLY) {
        if (!glIsTextureHandleResidentARB(resident_id)) {
            glMakeTextureHandleResidentARB(resident_id);
        }

        if (image_barrier.old_state == ResourceState::UNORDERED_ACCESS) {
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }
    else if (image_barrier.new_state == ResourceState::RENDER_TARGET ||
             image_barrier.new_state == ResourceState::DEPTH_READ) {

        if (glIsTextureHandleResidentARB(resident_id)) {
            glMakeTextureHandleNonResidentARB(resident_id);
        }
    }
    else if (image_barrier.old_state == ResourceState::RENDER_TARGET &&
             image_barrier.new_state == ResourceState::UNORDERED_ACCESS) {

        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    }
}

void fe::RendererOpenGL::handleCommand(const render_graph::BufferBarrier& buffer_barrier) {
}

void fe::RendererOpenGL::handleCommand(const render_graph::BeginRenderPass& begin_render_pass) {
    if (begin_render_pass.is_to_screen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(begin_render_pass.viewport.offset.x,
                   begin_render_pass.viewport.offset.y,
                   begin_render_pass.viewport.extent.x,
                   begin_render_pass.viewport.extent.y);

        GLbitfield clear_mask{};
        if (begin_render_pass.is_clears_color) {
            glClearColor(begin_render_pass.clear_color_value.r,
                         begin_render_pass.clear_color_value.g,
                         begin_render_pass.clear_color_value.b,
                         begin_render_pass.clear_color_value.a);
            clear_mask |= GL_COLOR_BUFFER_BIT;
        }

        if (begin_render_pass.is_clears_depth) {
            glClearDepth(begin_render_pass.clear_depth_value);
            clear_mask |= GL_DEPTH_BUFFER_BIT;
        }

        if (clear_mask != 0) glClear(clear_mask);

        return;
    }

    GLuint framebuffer_raw{};

    uint64_t framebuffer_hash = render_graph::color_depth_targets_hash(begin_render_pass.color_targets,
                                                                       begin_render_pass.color_targets_count,
                                                                       begin_render_pass.depth_target);

    auto it = m_FramebuffersCache.find(framebuffer_hash);
    if (it != m_FramebuffersCache.end()) {
        framebuffer_raw = it->second.get();
    }
    else {
        glCreateFramebuffers(1, &framebuffer_raw);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_raw);

        for (size_t i = 0; i < begin_render_pass.color_targets_count; i++) {
            size_t               texture_index  = begin_render_pass.color_targets[i];
            const OpenGLTexture& opengl_texture = m_OpenGLResourceManager.GetImage(texture_index);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, opengl_texture.texture, 0);
        }

        if (begin_render_pass.has_depth_target) {
            const OpenGLTexture& opengl_texture = m_OpenGLResourceManager.GetImage(begin_render_pass.depth_target);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, opengl_texture.texture, 0);
        }

        std::vector<GLenum> attachments{};

        for (size_t i = 0; i < begin_render_pass.color_targets_count; i++) {
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
    glViewport(0, 0, begin_render_pass.viewport.extent.x, begin_render_pass.viewport.extent.y);

    GLbitfield clear_mask{};
    if (begin_render_pass.is_clears_color) {
        glClearColor(begin_render_pass.clear_color_value.r,
                     begin_render_pass.clear_color_value.g,
                     begin_render_pass.clear_color_value.b,
                     begin_render_pass.clear_color_value.a);
        clear_mask |= GL_COLOR_BUFFER_BIT;
    }

    if (begin_render_pass.is_clears_depth) {
        glClearDepth(begin_render_pass.clear_depth_value);
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (clear_mask != 0) glClear(clear_mask);
}

void fe::RendererOpenGL::handleCommand(const render_graph::EndRenderPass& end_render_pass) {
    glBindVertexArray(0);
    glUseProgram(0);
}

void fe::RendererOpenGL::handleCommand(const render_graph::DrawIndexed& draw_indices) {
    glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES,
                                                  draw_indices.index_count,
                                                  GL_UNSIGNED_INT,
                                                  reinterpret_cast<void*>(static_cast<uintptr_t>(draw_indices.first_index * sizeof(uint32_t))),
                                                  draw_indices.instance_count,
                                                  draw_indices.vertex_offset,
                                                  draw_indices.first_instance);
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindShaderProgram& bind_shader_program) {
    m_BoundShaderProgramPtr = bind_shader_program.shader_program_ptr;
}

void fe::RendererOpenGL::handleCommand(const render_graph::BindMaterial& bind_material) {
    const OpenGLPipeline& pipeline = m_OpenGLResourceManager.GetOrCreatePipeline(m_BoundShaderProgramPtr, bind_material.material_ptr);
    bindPipeline(pipeline);
}
