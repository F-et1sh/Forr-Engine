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
                                   size_t              primary_window_index)
    : m_PlatformSystem(platform_system),
      m_PrimaryWindow(m_PlatformSystem.getWindow(primary_window_index)) {

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
}

fe::RendererOpenGL::~RendererOpenGL() {
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

    auto& mesh = m_GPUResourceManager.getMesh(index);

    mesh.vao.bind();
    glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_BYTE, 0);
    mesh.vao.unbind();

    m_Shader.unbind();
}
