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
      m_ResourceManager(resource_manager) {

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
    m_Camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    m_Camera.setRotation(glm::vec3(0.0f));
    m_Camera.setFlipY(false);

    float fov    = 60.0f;
    float aspect = (float) m_PrimaryWindow.getWidth() / (float) m_PrimaryWindow.getHeight();
    float znear  = 1.0f;
    float zfar   = 1000.0f;
    m_Camera.setPerspective(fov, aspect, znear, zfar);

    m_Shader.bind();

    glCreateBuffers(1, &ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    ShaderData shader_data{};
    shader_data.projection_matrix = m_Camera.getPerspectiveMatrix();
    shader_data.view_matrix       = m_Camera.getViewMatrix();
    shader_data.model_matrix      = glm::mat4(1.0f);

    glNamedBufferData(ubo, sizeof(shader_data), &shader_data, GL_DYNAMIC_DRAW);
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

void fe::RendererOpenGL::Draw(MeshID index) {
    m_Shader.bind();

    {
        m_Camera.translate(glm::vec3(0, 0, -0.025));
        m_Camera.rotate(glm::vec3(0, 1, 0));

        ShaderData shader_data{};
        shader_data.projection_matrix = m_Camera.getPerspectiveMatrix();
        shader_data.view_matrix       = m_Camera.getViewMatrix();
        shader_data.model_matrix      = glm::mat4(1.0f);

        glNamedBufferSubData(ubo, 0, sizeof(shader_data), &shader_data);
    }

    /*for (size_t i = 0; i < 8; i++) {
        auto& mesh = m_OpenGLResourceManager.getMesh(i);
        for (const auto& primitive : mesh.primitives) {
            primitive.vao.Bind();
            glDrawElements(GL_TRIANGLES, primitive.index_count, GL_UNSIGNED_INT, 0);
            primitive.vao.Unbind();
        }
    }*/

    m_Shader.unbind();
}

void fe::RendererOpenGL::InitializeGPUResources() {
    m_ResourceManager.RunForEach<resource::Texture>([&](const resource::Texture& texture) {
        m_OpenGLResourceManager.CreateTexture(texture);

        fe::logging::info("Loaded texture's size : %i %i", texture.width, texture.height);
    });
}
