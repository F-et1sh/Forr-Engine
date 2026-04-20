/*===============================================

    Forr Engine

    File : RendererOpenGL.cpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "RendererOpenGL.hpp"

static unsigned int ubo{};  // temp
static unsigned int ubo2{}; // temp

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

    //std::filesystem::path shader_path = PATH.getShadersPath() / "default";
    //m_Shader.LoadShader(shader_path);

    //std::filesystem::path vertex_shader_path   = PATH.getShadersPath() / L"default3.vert";
    //std::filesystem::path fragment_shader_path = PATH.getShadersPath() / L"default2.frag";

    //m_Shader.LoadShader(vertex_shader_path, fragment_shader_path);

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

    //m_Shader.bind();

    //glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLuint opengl_scene_data_ssbo{};

    glCreateBuffers(1, &opengl_scene_data_ssbo);
    glNamedBufferStorage(opengl_scene_data_ssbo, sizeof(m_ShaderData), &m_ShaderData, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, opengl_scene_data_ssbo);

    m_SceneSSBO.attach(opengl_scene_data_ssbo);
}

fe::RendererOpenGL::~RendererOpenGL() {
    //glDeleteBuffers(1, &ubo);
    //glDeleteBuffers(1, &ubo2);
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

    m_ShaderData.projection_matrix = m_Camera.getPerspectiveMatrix();
    m_ShaderData.view_matrix       = m_Camera.getViewMatrix();
}

void fe::RendererOpenGL::Draw(DrawMeshCommand command) {

    // TODO : wtf ?
    // Meshes also can have thier own transform, but only models can
    // Use resource::Model::Node::local/global_matrix

    auto        gpu_ptr = m_OpenGLResourceManager.GetGPUPointer(command.model_ptr);
    const auto& model   = *m_OpenGLResourceManager.GetResource(gpu_ptr);

    size_t model_index               = m_I;
    m_ShaderData.model_matrices[m_I] = command.transform;

    for (auto mesh_pointer : model.pointers_mesh) {
        const auto& mesh = *m_OpenGLResourceManager.GetResource(mesh_pointer);

        for (const auto& primitive : mesh.primitives) {
            auto        gpu_material_ptr = m_OpenGLResourceManager.GetGPUPointer(primitive.material_ptr);
            const auto& gpu_material     = *m_OpenGLResourceManager.GetResource(gpu_material_ptr);
            const auto& material         = m_ResourceManager.GetResource(primitive.material_ptr);

            const auto& shader = *m_OpenGLResourceManager.GetResource(gpu_material.shader_program_ptr);

            glUseProgram(shader.shader_program);

            glBindVertexArray(mesh.vao);

            glNamedBufferSubData(m_SceneSSBO, 0, sizeof(m_ShaderData), &m_ShaderData);

            //glNamedBufferSubData(ubo2, 0, sizeof(glm::vec3), material->buffer.data());

            auto location = glGetUniformLocation(shader.shader_program, "model_index");
            glUniform1i(location, model_index);

            glDrawElements(GL_TRIANGLES, primitive.index_count, GL_UNSIGNED_INT, (void*) primitive.index_offset);

            glBindVertexArray(0);

            glUseProgram(0);
        }
    }

    m_I++;
}

void fe::RendererOpenGL::EndFrame() {
    glfwSwapBuffers(m_GLFWwindow);

    m_I = 0;
}

void fe::RendererOpenGL::InitializeGPUResources() {
    m_ResourceManager.RunForEach<resource::Texture>([&](const resource::Texture&       texture,
                                                        fe::pointer<resource::Texture> texture_ptr) {
        m_OpenGLResourceManager.CreateResource(texture_ptr);

        fe::logging::info("Loaded texture's size : %i %i", texture.width, texture.height);
    });

    m_ResourceManager.RunForEach<resource::Material>([&](const resource::Material&       material,
                                                         fe::pointer<resource::Material> material_ptr) {
        auto        gpu_material_ptr = m_OpenGLResourceManager.CreateResource(material_ptr);
        const auto& gpu_material     = m_OpenGLResourceManager.GetResource(gpu_material_ptr);

        //const auto& shader_program = m_OpenGLResourceManager.GetResource(gpu_material->shader_program_ptr);

        //glUseProgram(shader_program->shader_program);

        //{
        //    glCreateBuffers(1, &ubo);
        //    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

        //    ShaderData shader_data{};
        //    shader_data.projection_matrix = m_Camera.getPerspectiveMatrix();
        //    shader_data.view_matrix       = m_Camera.getViewMatrix();
        //    //shader_data.model_matrix      = glm::mat4(1.0f);

        //    glNamedBufferData(ubo, sizeof(shader_data), &shader_data, GL_DYNAMIC_DRAW);

        //    {
        //        glCreateBuffers(1, &ubo2);
        //        glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo2);

        //        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        //        glNamedBufferData(ubo2, sizeof(glm::vec3), &color, GL_DYNAMIC_DRAW);
        //    }
        //}

        //glUseProgram(0);

        //assert(material.buffer.data() != nullptr);

        //fe::logging::info("Loaded material's color : %f %f %f", material.color.x, material.color.y, material.color.z);
    });

    m_ResourceManager.RunForEach<resource::Model>([&](const resource::Model&       model,
                                                      fe::pointer<resource::Model> model_ptr) {
        m_OpenGLResourceManager.CreateResource(model_ptr);

        fe::logging::info("Loaded model's mesh count %i", model.meshes.size());
    });
}
