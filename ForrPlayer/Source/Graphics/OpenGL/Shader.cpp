/*===============================================

    Forr Engine

    File : Shader.cpp
    Role : OpenGL Shader

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Shader.hpp"

fe::Shader::~Shader() {
    glDeleteProgram(m_Program);
}

void fe::Shader::createProgram() noexcept {
    m_Program = glCreateProgram();
}

bool fe::Shader::loadFromSource(const char* source, const unsigned int& type) const {
    // Vertex or Fragment Shader
    unsigned int shader = glCreateShader(type);

    // Load Source to Shader
    glShaderSource(shader, 1, &source, nullptr);
    // Compile the Shader
    glCompileShader(shader);
    // Bind the Shader to the Program
    glAttachShader(m_Program, shader);
    // Delete the Shader
    glDeleteShader(shader);

    // Check Shader Errors
    int result = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*) _malloca(length * sizeof(char));
        glGetShaderInfoLog(shader, length, &length, message);

        std::wcerr << "ERROR : Failed to Compile Shader\n"
                   << message << '\n'
                   << '\n';

        glDeleteShader(shader);

        return false;
    }

    return true;
}

bool fe::Shader::loadFromFile(const std::filesystem::path& path, const unsigned int& type) {
    std::ifstream file(path);
    if (!file.good()) {
        std::wcerr << L"ERROR: Failed to open shader file\nPath : " << path.c_str() << '\n'
                   << '\n';
        return false;
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    if (source.empty()) {
        std::wcerr << L"ERROR: Shader file is empty\nPath: " << path.c_str() << '\n'
                   << '\n';
        return false;
    }

    return loadFromSource(source.c_str(), type);
}

void fe::Shader::linkShader() const {
    glLinkProgram(m_Program);
    glValidateProgram(m_Program);
}

void fe::Shader::LoadShader(const std::filesystem::path& file_path) {
    this->createProgram();

    const std::filesystem::path& path = file_path;

    const std::filesystem::path vertex_path   = path.wstring() + L".vert";
    const std::filesystem::path fragment_path = path.wstring() + L".frag";

    const bool vert_init = this->loadFromFile(vertex_path, fe::Shader::VERTEX);
    const bool frag_init = this->loadFromFile(fragment_path, fe::Shader::FRAGMENT);

    if (vert_init && frag_init) {
        this->linkShader();
        return;
    }

    // if user will use path with an extension, program anyway can work
    // Example : SomeShader.shader ==> SomeShader.shader.vert & SomeShader.shader.frag
    // Else -> throw an error

    if (path.has_extension()) {
        fe::logging::error("Failed to Initialize a shader\n \
                            You tried to use one path for both shaders (vertex and fragment)\n \
                            Delete the extension of the path\n \
                            Source Path : %s\n \
                            Vertex Path : %s\n \
                            Fragment Path : %s",
                           path.string().c_str(),
                           vertex_path.string().c_str(),
                           fragment_path.string().c_str());
    }
}

void fe::Shader::LoadShader(const std::filesystem::path& vert_path, const std::filesystem::path& frag_path) {
    this->createProgram();

    const std::filesystem::path& vertex_path   = vert_path;
    const std::filesystem::path& fragment_path = frag_path;

    bool vert_init = this->loadFromFile(vertex_path, fe::Shader::VERTEX);
    bool frag_init = this->loadFromFile(fragment_path, fe::Shader::FRAGMENT);

    if (vert_init && frag_init) {
        this->linkShader();
        return;
    }

    fe::logging::error("Failed to Initialize a shader.\n \
                        Source Path : %s\n \
                        Vertex Path : %s\n \
                        Fragment Path : %s",
                       vertex_path.string().c_str(),
                       fragment_path.string().c_str());
}

void fe::Shader::bind() const noexcept {
    glUseProgram(m_Program);
}
void fe::Shader::unbind() noexcept {
    glUseProgram(0);
}

#pragma region Legacy Uniform Setting

// Methods with Uniform Setting
#pragma region Set Uniforms

void fe::Shader::setUniformInt(const char* uniform_name, int load_uniform) const noexcept {
    glUniform1i(glGetUniformLocation(this->m_Program, uniform_name), load_uniform);
}
void fe::Shader::setUniformIntArray(const char* uniform_name, int* load_uniform, unsigned int count) const noexcept {
    glUniform1iv(glGetUniformLocation(this->m_Program, uniform_name), count, load_uniform);
}

void fe::Shader::setUniformFloat(const char* uniform_name, float load_uniform) const noexcept {
    glUniform1f(glGetUniformLocation(this->m_Program, uniform_name), load_uniform);
}
void fe::Shader::setUniformFloatArray(const char* uniform_name, float* load_uniform, unsigned int count) const noexcept {
    glUniform1fv(glGetUniformLocation(this->m_Program, uniform_name), count, load_uniform);
}

void fe::Shader::setUniformVec2(const char* uniform_name, const glm::vec2& load_uniform) const noexcept {
    glUniform2f(glGetUniformLocation(this->m_Program, uniform_name), load_uniform.x, load_uniform.y);
}
void fe::Shader::setUniformVec2Array(const char* uniform_name, const glm::vec2* load_uniform, unsigned int count) const noexcept {
    glUniform2fv(glGetUniformLocation(this->m_Program, uniform_name), count, &load_uniform[0].x);
}

void fe::Shader::setUniformVec3(const char* uniform_name, const glm::vec3& load_uniform) const noexcept {
    glUniform3f(glGetUniformLocation(this->m_Program, uniform_name), load_uniform.x, load_uniform.y, load_uniform.z);
}
void fe::Shader::setUniformVec3Array(const char* uniform_name, const glm::vec3* load_uniform, unsigned int count) const noexcept {
    glUniform3fv(glGetUniformLocation(this->m_Program, uniform_name), count, &load_uniform[0].x);
}

void fe::Shader::setUniformVec4(const char* uniform_name, const glm::vec4& load_uniform) const noexcept {
    glUniform4f(glGetUniformLocation(this->m_Program, uniform_name), load_uniform.x, load_uniform.y, load_uniform.z, load_uniform.w);
}
void fe::Shader::setUniformVec4Array(const char* uniform_name, const glm::vec4* load_uniform, unsigned int count) const noexcept {
    glUniform4fv(glGetUniformLocation(this->m_Program, uniform_name), count, &load_uniform[0].x);
}

void fe::Shader::setUniformMat3(const char* uniform_name, const glm::mat3& load_uniform) const noexcept {
    glUniformMatrix3fv(glGetUniformLocation(this->m_Program, uniform_name), 1, GL_FALSE, glm::value_ptr(load_uniform));
}
void fe::Shader::setUniformMat3Array(const char* uniform_name, const glm::mat3* load_uniform, unsigned int count) const noexcept {
    glUniformMatrix3fv(glGetUniformLocation(this->m_Program, uniform_name), count, GL_FALSE, glm::value_ptr(load_uniform[0]));
}

void fe::Shader::setUniformMat4(const char* uniform_name, const glm::mat4& load_uniform) const noexcept {
    glUniformMatrix4fv(glGetUniformLocation(this->m_Program, uniform_name), 1, GL_FALSE, glm::value_ptr(load_uniform));
}
void fe::Shader::setUniformMat4Array(const char* uniform_name, const glm::mat4* load_uniform, unsigned int count) const noexcept {
    glUniformMatrix4fv(glGetUniformLocation(this->m_Program, uniform_name), count, GL_FALSE, glm::value_ptr(load_uniform[0]));
}

#pragma endregion

// Static Methods with Uniform Setting
#pragma region Set Uniforms

void fe::Shader::setUniformInt(unsigned int shader_ref, const char* uniform_name, int load_uniform) noexcept {
    glUniform1i(glGetUniformLocation(shader_ref, uniform_name), load_uniform);
}
void fe::Shader::setUniformIntArray(unsigned int shader_ref, const char* uniform_name, int* load_uniform, unsigned int count) noexcept {
    glUniform1iv(glGetUniformLocation(shader_ref, uniform_name), count, load_uniform);
}

void fe::Shader::setUniformFloat(unsigned int shader_ref, const char* uniform_name, float load_uniform) noexcept {
    glUniform1f(glGetUniformLocation(shader_ref, uniform_name), load_uniform);
}
void fe::Shader::setUniformFloatArray(unsigned int shader_ref, const char* uniform_name, float* load_uniform, unsigned int count) noexcept {
    glUniform1fv(glGetUniformLocation(shader_ref, uniform_name), count, load_uniform);
}

void fe::Shader::setUniformVec2(unsigned int shader_ref, const char* uniform_name, const glm::vec2& load_uniform) noexcept {
    glUniform2f(glGetUniformLocation(shader_ref, uniform_name), load_uniform.x, load_uniform.y);
}
void fe::Shader::setUniformVec2Array(unsigned int shader_ref, const char* uniform_name, const glm::vec2* load_uniform, unsigned int count) noexcept {
    glUniform2fv(glGetUniformLocation(shader_ref, uniform_name), count, &load_uniform[0].x);
}

void fe::Shader::setUniformVec3(unsigned int shader_ref, const char* uniform_name, const glm::vec3& load_uniform) noexcept {
    glUniform3f(glGetUniformLocation(shader_ref, uniform_name), load_uniform.x, load_uniform.y, load_uniform.z);
}
void fe::Shader::setUniformVec3Array(unsigned int shader_ref, const char* uniform_name, const glm::vec3* load_uniform, unsigned int count) noexcept {
    glUniform3fv(glGetUniformLocation(shader_ref, uniform_name), count, &load_uniform[0].x);
}

void fe::Shader::setUniformVec4(unsigned int shader_ref, const char* uniform_name, const glm::vec4& load_uniform) noexcept {
    glUniform4f(glGetUniformLocation(shader_ref, uniform_name), load_uniform.x, load_uniform.y, load_uniform.z, load_uniform.w);
}
void fe::Shader::setUniformVec4Array(unsigned int shader_ref, const char* uniform_name, const glm::vec4* load_uniform, unsigned int count) noexcept {
    glUniform4fv(glGetUniformLocation(shader_ref, uniform_name), count, &load_uniform[0].x);
}

void fe::Shader::setUniformMat3(unsigned int shader_ref, const char* uniform_name, const glm::mat3& load_uniform) noexcept {
    glUniformMatrix3fv(glGetUniformLocation(shader_ref, uniform_name), 1, GL_FALSE, glm::value_ptr(load_uniform));
}
void fe::Shader::setUniformMat3Array(unsigned int shader_ref, const char* uniform_name, const glm::mat3* load_uniform, unsigned int count) noexcept {
    glUniformMatrix3fv(glGetUniformLocation(shader_ref, uniform_name), count, GL_FALSE, glm::value_ptr(load_uniform[0]));
}

void fe::Shader::setUniformMat4(unsigned int shader_ref, const char* uniform_name, const glm::mat4& load_uniform) noexcept {
    glUniformMatrix4fv(glGetUniformLocation(shader_ref, uniform_name), 1, GL_FALSE, glm::value_ptr(load_uniform));
}
void fe::Shader::setUniformMat4Array(unsigned int shader_ref, const char* uniform_name, const glm::mat4* load_uniform, unsigned int count) noexcept {
    glUniformMatrix4fv(glGetUniformLocation(shader_ref, uniform_name), count, GL_FALSE, glm::value_ptr(load_uniform[0]));
}

#pragma endregion

#pragma endregion
