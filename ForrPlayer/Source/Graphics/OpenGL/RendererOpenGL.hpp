/*===============================================

    Forr Engine

    File : RendererOpenGL.hpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IRenderer.hpp"
#include "Graphics/Camera.hpp"

#include "OpenGLResourceManagement/OpenGLResourceManager.hpp"

#include "Shader.hpp"
#include <GLFW/glfw3.h>

namespace fe {
    class RendererOpenGL : public IRenderer {
    public:
        RendererOpenGL(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index, ResourceManager& resource_manager);
        ~RendererOpenGL();

        void SetClearColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) override;

        void BeginFrame() override;
        void Draw(DrawMeshCommand command) override;
        void EndFrame() override;

        void InitializeGPUResources() override;

    private:
        ResourceManager& m_ResourceManager;

        Shader m_Shader;

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        GLFWwindow* m_GLFWwindow;

        OpenGLResourceManager m_OpenGLResourceManager;
        Camera                m_Camera{}; // temp
    };
} // namespace fe
