/*===============================================

    Forr Engine

    File : WindowGLFW.hpp
    Role : GLFW window implemetation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Platform/IWindow.hpp"
#include "Forr/Core/Events/EventBus.hpp"

#include <GLFW/glfw3.h>

namespace fe {
    class WindowGLFW : public IWindow {
    public:
        WindowGLFW(const WindowDesc& desc, EventBus& event_bus);
        ~WindowGLFW() override;

        FORR_NODISCARD bool IsOpen() override;
        void                PollEvents() override;

        FORR_NODISCARD void* getNativeHandle() override { return static_cast<void*>(m_GLFWwindow); }
        FORR_NODISCARD int   getWidth() override { return m_Description.width; }
        FORR_NODISCARD int   getHeight() override { return m_Description.height; }
        FORR_NODISCARD std::string getName() override { return m_Description.name; }
        FORR_NODISCARD int         getMonitorIndex() override { return m_Description.monitor_index; }
        FORR_NODISCARD int         getVSync() override { return m_Description.vsync; }
        FORR_NODISCARD bool        getIsFullscreen() override { return m_Description.is_fullscreen; }

        void setResolution(int width, int height) override;

    private:
        void centralizeWindow() const;

    private:
        static void windowSizeCallback(GLFWwindow* window_glfw, int width, int height);
        static void keyCallback(GLFWwindow* window_glfw, int key, int scancode, int action, int mods);
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    private:
        WindowDesc m_Description{};

        std::reference_wrapper<EventBus> m_EventBus;

        GLFWwindow*  m_GLFWwindow{};
        GLFWmonitor* m_GLFWmonitor{};
    };
} // namespace fe
