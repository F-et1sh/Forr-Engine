/*===============================================

    Forr Engine

    File : ShaderOpenGL.cpp
    Role : OpenGL Shader

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "ShaderOpenGL.hpp"

#include <fstream>

void fe::ShaderOpenGL::Initialize(ShaderDesc& desc) {
    m_Program = glCreateProgram();

    std::filesystem::path vertex_full_path   = PATH.getShadersPath() / desc.vertex_relative_path;
    std::filesystem::path fragment_full_path = PATH.getShadersPath() / desc.fragment_relative_path;

    this->attachVertexShader(vertex_full_path);
    this->attachFragmentShader(desc.fragment_relative_path);

    glLinkProgram(m_Program);
    glValidateProgram(m_Program);
}

void fe::ShaderOpenGL::Bind() {
    glUseProgram(m_Program);
}

void fe::ShaderOpenGL::Unbind() {
    glUseProgram(0);
}

void fe::ShaderOpenGL::loadSource(const std::filesystem::path& full_path, const char* source_dst) {
    std::ifstream file(full_path);
    if (!file.good()) {
        fe::logging::error("Failed to open shader file in OpenGL\nPath : %s", full_path.string().c_str());
        return;
    }

    source_dst = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()).c_str();
}

void fe::ShaderOpenGL::attachVertexShader(const std::filesystem::path& full_path) {
    const char* source{};

    this->loadSource(full_path, source);

    GLuint shader{};
    shader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    glAttachShader(m_Program, shader);

    GLint result{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    
    if (result == GL_FALSE) {
        int length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) _malloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);

        fe::logging::error("Failed to compile vertex shader in OpenGL\nMessage : %s\nPath : %s", message, full_path.c_str());
    }

    glDeleteShader(shader);
}

void fe::ShaderOpenGL::attachFragmentShader(const std::filesystem::path& full_path) {
    const char* source{};

    this->loadSource(full_path, source);

    GLuint shader{};
    shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    glAttachShader(m_Program, shader);

    GLint result{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    
    if (result == GL_FALSE) {
        int length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) _malloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);

        fe::logging::error("Failed to compile vertex shader in OpenGL\nMessage : %s\nPath : %s", message, full_path.c_str());
    }

    glDeleteShader(shader);
}
