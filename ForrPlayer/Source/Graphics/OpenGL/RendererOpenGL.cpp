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

    this->createSceneDataSSBO();
}

fe::RendererOpenGL::~RendererOpenGL() {
    glFinish();
}

void fe::RendererOpenGL::SetClearColor(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
}

void fe::RendererOpenGL::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    { // temp
        if (glfwGetKey(m_GLFWwindow, GLFW_KEY_A))
            m_Camera.translate(glm::vec3(1.0f, 0.0f, 0.0f));
        else if (glfwGetKey(m_GLFWwindow, GLFW_KEY_D))
            m_Camera.translate(glm::vec3(-1.0f, 0.0f, 0.0f));

        if (glfwGetKey(m_GLFWwindow, GLFW_KEY_W))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, 1.0f));
        else if (glfwGetKey(m_GLFWwindow, GLFW_KEY_S))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, -1.0f));
    }

    m_SceneData.projection_matrix = m_Camera.getPerspectiveMatrix();
    m_SceneData.view_matrix       = m_Camera.getViewMatrix();
}

void fe::RendererOpenGL::Draw(DrawMeshCommand command) {
    const auto& model = m_ResourceManager.GetResource(command.model_ptr);
    //const auto& gpu_model = m_OpenGLResourceManager.GetResource(model->gpu_handle); // TODO : this must work

    this->increaseMeshIndex();
}

void fe::RendererOpenGL::EndFrame() {
    glfwSwapBuffers(m_GLFWwindow);
    this->resetMeshIndex();
}

void fe::RendererOpenGL::InitializeGPUResources() {
    m_ResourceManager.RunForEach<resource::Texture>([&](const resource::Texture&       texture,
                                                        fe::pointer<resource::Texture> texture_ptr) {
        m_OpenGLResourceManager.CreateResource(texture);

        fe::logging::info("Loaded texture's size : %i %i", texture.width, texture.height);
    });

    m_ResourceManager.RunForEach<resource::Material>([&](const resource::Material&       material,
                                                         fe::pointer<resource::Material> material_ptr) {
        m_OpenGLResourceManager.CreateResource(material_ptr);
    });

    m_ResourceManager.RunForEach<resource::Model>([&](const resource::Model&       model,
                                                      fe::pointer<resource::Model> model_ptr) {
        m_OpenGLResourceManager.CreateResource(model_ptr);

        fe::logging::info("Loaded model's mesh count %i", model.meshes.size());
    });
}

void fe::RendererOpenGL::createSceneDataSSBO() {
    GLuint opengl_scene_data_ssbo{};

    glCreateBuffers(1, &opengl_scene_data_ssbo);
    glNamedBufferStorage(opengl_scene_data_ssbo, sizeof(m_SceneData), &m_SceneData, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, opengl_scene_data_ssbo);

    m_SceneSSBO.attach(opengl_scene_data_ssbo);
}
