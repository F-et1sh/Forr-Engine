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

void fe::RendererOpenGL::CreateGPUResources(const render_graph::CreateCommandList& create_command_list) {
    for ()
}

void fe::RendererOpenGL::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_FrameData[m_CurrentFrame].sync) {
        glClientWaitSync(m_FrameData[m_CurrentFrame].sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    }
}

void fe::RendererOpenGL::EndFrame(const render_graph::RenderCommandList& render_command_list) {
    for (const auto& render_command : render_command_list) {
        
    }

    //this->handleRenderQueue(render_packet);

    m_FrameData[m_CurrentFrame].sync.reset();
    GLsync sync_raw = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    m_FrameData[m_CurrentFrame].sync.attach(sync_raw);

    glBindVertexArray(0);
    glUseProgram(0);

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

    m_ResourceManager.RunForEach<resource::Material>([&](resource::Material& material) {
        m_OpenGLResourceManager.CreateResource(material);
    });

    //m_ResourceManager.RunForEach<resource::ShaderProgram>([&](resource::ShaderProgram& shader_program) {
        //m_OpenGLResourceManager.CreateResource(shader_program);
    //});

    m_ResourceManager.RunForEach<resource::Model>([&](resource::Model& model) {
        m_OpenGLResourceManager.CreateResource(model);

        fe::logging::info("Loaded model's mesh count %i", model.meshes.size());
    });
}

void fe::RendererOpenGL::handleRenderQueue(const RenderPacket& render_packet) {
    //uint8_t* materials_data = m_ResourceManager.GetMaterialsData();


    constexpr static std::size_t object_binding_index = 0;
    constexpr static std::size_t lights_binding_index = 1;

    auto& storage_buffer = m_FrameData[m_CurrentFrame].storage_buffer;

    { // temp
        auto glfw_window = (GLFWwindow*) m_PrimaryWindow.getNativeHandle();

        float speed = 0.025f;

        if (glfwGetKey(glfw_window, GLFW_KEY_A))
            m_Camera.translate(glm::vec3(speed, 0.0f, 0.0f));
        else if (glfwGetKey(glfw_window, GLFW_KEY_D))
            m_Camera.translate(glm::vec3(-speed, 0.0f, 0.0f));

        if (glfwGetKey(glfw_window, GLFW_KEY_W))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, speed));
        else if (glfwGetKey(glfw_window, GLFW_KEY_S))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, -speed));

        auto* object_ptr = static_cast<uint8_t*>(storage_buffer.bindings[object_binding_index].mapped);

        struct GPUCamera {
            glm::mat4 p;
            glm::mat4 v;
        } cam{ m_Camera.getPerspectiveMatrix(), m_Camera.getViewMatrix() };
        memcpy(object_ptr, &cam, sizeof(cam));
        object_ptr += sizeof(cam);

        if (!render_packet.object_transforms.empty()) {
            size_t bytes_to_copy = render_packet.object_transforms.size() * sizeof(glm::mat4);
            memcpy(object_ptr, render_packet.object_transforms.data(), bytes_to_copy);
        }

        auto*    lights_ptr   = static_cast<uint8_t*>(storage_buffer.bindings[lights_binding_index].mapped);
        uint32_t lights_count = render_packet.lights.size();
        memcpy(lights_ptr, &lights_count, sizeof(lights_count));
        lights_ptr += 16;
        if (!render_packet.lights.empty()) {
            size_t bytes_to_copy = render_packet.lights.size() * sizeof(GPULight);
            memcpy(lights_ptr, render_packet.lights.data(), bytes_to_copy);
        }
    }

    GLuint object_buffer_raw = storage_buffer.bindings[object_binding_index].buffer.get();
    GLuint light_buffer_raw  = storage_buffer.bindings[lights_binding_index].buffer.get();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, object_binding_index, object_buffer_raw);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, lights_binding_index, light_buffer_raw);

    // draw
    for (const auto& draw_command : render_packet.draw_commands) {
        //const auto& material              = *m_ResourceManager.GetResource(draw_command.material_ptr);
        //const auto& shader_program        = *m_ResourceManager.GetResource(material.);
        //const auto& opengl_material       = m_OpenGLResourceManager.GetResource(material.gpu_handle);
        //const auto& opengl_shader_program = m_OpenGLResourceManager.GetResource(shader_program.gpu_handle);

        // bind shader ( material )
        //if (material.gpu_handle != m_CurrentMaterial) {
        //    m_CurrentMaterial = material.gpu_handle;

        //    glUseProgram(opengl_shader_program.shader_program);

        //    const auto& material_bindings = opengl_shader_program.shader_buffersz.bindings;
        //    for (std::size_t i = 0; i < material_bindings.size(); i++) {
        //        const auto&       binding       = material_bindings[i];
        //        const std::size_t binding_index = 0;

        //        if (!material.buffer.empty()) {
        //            memcpy(binding.mapped, material.buffer.data(), material.buffer.size());
        //        }
        //        GLuint buffer_raw = binding.buffer.get();
        //        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_index, buffer_raw);
        //    }
        //}

        // bind vertex buffer
        if (draw_command.mesh_handle != m_CurrentMesh) {
            m_CurrentMesh = draw_command.mesh_handle;

            const auto& opengl_mesh = m_OpenGLResourceManager.GetResource(draw_command.mesh_handle);
            glBindVertexArray(opengl_mesh.vao);
        }

        glUniform1i(0, draw_command.instance_index);

        glDrawElements(GL_TRIANGLES, draw_command.index_count, GL_UNSIGNED_INT, (void*) draw_command.index_offset);
    }
}
