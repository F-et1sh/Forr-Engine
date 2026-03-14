/*===============================================

    Forr Engine

    File : RendererOpenGL.cpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "RendererOpenGL.hpp"

static unsigned int ubo{};

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

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fe::logging::error("Failed to initialize OpenGL context");
        return;
    }

    glViewport(0, 0, m_PrimaryWindow.getWidth(), m_PrimaryWindow.getHeight());
    glEnable(GL_DEPTH_TEST);

    std::filesystem::path shader_path = PATH.getShadersPath() / "default";
    m_Shader.LoadShader(shader_path);

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

    m_Shader.bind();

    glCreateBuffers(1, &ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    ShaderData shader_data{};
    shader_data.projection_matrix = m_Camera.getPerspectiveMatrix();
    shader_data.view_matrix       = m_Camera.getViewMatrix();
    shader_data.model_matrix      = glm::mat4(1.0f);

    glNamedBufferData(ubo, sizeof(shader_data), &shader_data, GL_DYNAMIC_DRAW);

    glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

fe::RendererOpenGL::~RendererOpenGL() {
    glDeleteBuffers(1, &ubo);
    glFinish();
}

void fe::RendererOpenGL::ClearScreen(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void fe::RendererOpenGL::SwapBuffers() {
    glfwSwapBuffers(m_GLFWwindow);
}

void fe::RendererOpenGL::BeginFrame() {
    m_Shader.bind();

    { // temp
        if (glfwGetKey(m_GLFWwindow, GLFW_KEY_A))
            m_Camera.translate(glm::vec3(1.0f, 0.0f, 0.0f));
        else if (glfwGetKey(m_GLFWwindow, GLFW_KEY_D))
            m_Camera.translate(glm::vec3(-1.0f, 0.0f, 0.0f));

        if (glfwGetKey(m_GLFWwindow, GLFW_KEY_W))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, 1.0f));
        else if (glfwGetKey(m_GLFWwindow, GLFW_KEY_S))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, -1.0f));

        ShaderData shader_data{};
        shader_data.projection_matrix = m_Camera.getPerspectiveMatrix();
        shader_data.view_matrix       = m_Camera.getViewMatrix();
        shader_data.model_matrix      = glm::mat4(1.0f);

        glNamedBufferSubData(ubo, 0, sizeof(shader_data), &shader_data);
    }
}

void fe::RendererOpenGL::Draw(DrawMeshCommand command) {

    //auto cpu_mesh     = m_ResourceManager.GetResource(ptr);

    auto opengl_model = m_OpenGLResourceManager.GetResource<OpenGLModel>(command.model_ptr);

    for (auto mesh_pointer : opengl_model->pointers_mesh) {
        const auto& mesh_storage = m_OpenGLResourceManager.GetStorage<OpenGLMesh>();
        auto        mesh         = mesh_storage.get(mesh_pointer);

        for (const auto& primitive : mesh->primitives) {

            glBindVertexArray(primitive.vao);

            if (primitive.index_count > 0) {
                glDrawElements(GL_TRIANGLES, primitive.index_count, GL_UNSIGNED_INT, (void*) primitive.index_offset);
            }
            else {
                //glDrawArrays(GL_TRIANGLES, 0, cpu_mesh->meshes[0].primitives[0].vertices.size());
            }

            glBindVertexArray(0);
        }
    }
}

void fe::RendererOpenGL::EndFrame() {
    m_Shader.unbind();
}

void fe::RendererOpenGL::InitializeGPUResources() {
    m_ResourceManager.RunForEach<resource::Texture>([&](const resource::Texture&       texture,
                                                        fe::pointer<resource::Texture> texture_ptr) {
        m_OpenGLResourceManager.CreateTexture(texture_ptr);

        fe::logging::info("Loaded texture's size : %i %i", texture.width, texture.height);
    });

    m_ResourceManager.RunForEach<resource::Material>([&](const resource::Material& material) { // TODO : provide materials
        // ...
    });

    m_ResourceManager.RunForEach<resource::Model>([&](const resource::Model&       model,
                                                      fe::pointer<resource::Model> model_ptr) {
        m_OpenGLResourceManager.CreateModel(model_ptr);

        fe::logging::info("Loaded model's mesh count %i", model.meshes.size());
    });
}
