/*===============================================

    Forr Engine

    File : OpenGLResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "OpenGLResourceManager.hpp"

template <>
void fe::OpenGLResourceManager::CreateResource(fe::resource::Material& material) {
    OpenGLMaterial opengl_material{};

    auto vertex_shader   = m_ResourceManager.GetResource(material.vertex_shader_ptr);
    auto fragment_shader = m_ResourceManager.GetResource(material.fragment_shader_ptr);

    this->createShaderProgram(opengl_material, { vertex_shader, fragment_shader });

    m_StorageMaterials.emplace_back(std::move(opengl_material));

    material.gpu_handle.index = m_StorageMaterials.size() - 1;
}
template void fe::OpenGLResourceManager::CreateResource(fe::resource::Material& material);

///

template <>
void fe::OpenGLResourceManager::CreateResource(fe::resource::Model& model) {
    for (auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }
}
template void fe::OpenGLResourceManager::CreateResource(fe::resource::Model& model);

///

template<>
const fe::OpenGLMaterial& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Material> handle) const {
    return m_StorageMaterials[handle.index];
}
template fe::OpenGLMaterial& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Material> handle)const;

template<>
const fe::OpenGLShaderProgram& fe::OpenGLResourceManager::GetResource(GPUHandle<OpenGLShaderProgram> handle) const {
    return m_StorageShaderPrograms[handle.index];
}
template fe::OpenGLShaderProgram& fe::OpenGLResourceManager::GetResource(GPUHandle<OpenGLShaderProgram> handle)const;

template<>
const fe::OpenGLMesh& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Model::Mesh> handle) const {
    return m_StorageMeshes[handle.index];
}
template fe::OpenGLMesh& fe::OpenGLResourceManager::GetResource(GPUHandle<resource::Model::Mesh> handle)const;


///

fe::GPUHandle<fe::resource::Model::Mesh> fe::OpenGLResourceManager::createMesh(resource::Model::Mesh& mesh) {
    OpenGLMesh opengl_mesh{};

    GLuint vao{};
    GLuint vbo{};
    GLuint ebo{};

    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    constexpr GLsizei stride = sizeof(Vertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glCreateBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    opengl_mesh.primitives.reserve(mesh.primitives.size());

    for (const auto& primitive : mesh.primitives) {
        auto& opengl_primitive = opengl_mesh.primitives.emplace_back();

        opengl_primitive.index_count  = primitive.index_count;
        opengl_primitive.index_offset = primitive.index_offset;

        // clang-format off
        switch (primitive.render_mode) {
            case RenderMode::POINTS        : opengl_primitive .render_mode = GL_POINTS        ; break;
            case RenderMode::LINES         : opengl_primitive .render_mode = GL_LINES         ; break;
            case RenderMode::LINE_LOOP     : opengl_primitive .render_mode = GL_LINE_LOOP     ; break;
            case RenderMode::LINE_STRIP    : opengl_primitive .render_mode = GL_LINE_STRIP    ; break;
            case RenderMode::TRIANGLES     : opengl_primitive .render_mode = GL_TRIANGLES     ; break;
            case RenderMode::TRIANGLE_STRIP: opengl_primitive .render_mode = GL_TRIANGLE_STRIP; break;
            case RenderMode::TRIANGLE_FAN  : opengl_primitive .render_mode = GL_TRIANGLE_FAN  ; break;
            default:
                fe::logging::warning("Unified -> OpenGL. Unsupported render mode %i. Using GL_TRIANGLES as default", primitive.render_mode);
                opengl_primitive .render_mode = GL_TRIANGLES;
        }
        // clang-format on
    }

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    opengl_mesh.vao.attach(vao);
    opengl_mesh.vbo.attach(vbo);
    opengl_mesh.ebo.attach(ebo);

    m_StorageMeshes.emplace_back(std::move(opengl_mesh));
    mesh.gpu_handle.index = m_StorageMeshes.size() - 1;
    return mesh.gpu_handle;
}

fe::GPUHandle<fe::OpenGLShaderProgram> fe::OpenGLResourceManager::createShaderProgram(OpenGLMaterial& opengl_material, std::vector<resource::Shader*> shaders) {
    OpenGLShaderProgram opengl_shader_program{};
    GLuint              shader_program = glCreateProgram();

    for (size_t i = 0; i < shaders.size(); i++) {
        const auto& shader = shaders[i];

        unsigned int opengl_type{};
        unsigned int opengl_shader{};

        // clang-format off
        switch (shader->type) {
            case resource::Shader::Type::VERTEX: opengl_type = GL_VERTEX_SHADER; break;
            case resource::Shader::Type::FRAGMENT: opengl_type = GL_FRAGMENT_SHADER; break;
        }
        // clang-format on

        opengl_shader = glCreateShader(opengl_type);

        glShaderBinary(1, &opengl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, shader->source_code.data(), shader->source_code.size() * sizeof(uint32_t));
        glSpecializeShader(opengl_shader, "main", 0, nullptr, nullptr);

        glCompileShader(opengl_shader);

        int result = 0;
        glGetShaderiv(opengl_shader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            int length = 0;
            glGetShaderiv(opengl_shader, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*) _malloca(length * sizeof(char));
            glGetShaderInfoLog(opengl_shader, length, &length, message);

            fe::logging::error("Unified -> OpenGL. Failed to compile a shader\nMessage : %s", message);
        }
        else {
            glAttachShader(shader_program, opengl_shader);
        }

        glDeleteShader(opengl_shader);
    }
    opengl_shader_program.shader_program.attach(shader_program);

    m_StorageShaderPrograms.emplace_back(std::move(opengl_shader_program));

    opengl_material.shader_program_handle.index = m_StorageShaderPrograms.size() - 1;
    return opengl_material.shader_program_handle;
}
