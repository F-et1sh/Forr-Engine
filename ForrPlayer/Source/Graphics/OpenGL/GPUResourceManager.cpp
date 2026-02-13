/*===============================================

    Forr Engine

    File : GPUResourceManager.cpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "GPUResourceManager.hpp"

fe::MeshID fe::GPUResourceManager::CreateTriangle() {
    std::vector<Vertex> triangle_vertices{
        Vertex{ glm::vec3{ -0.5f, 0.5f, 0.0f } },
        Vertex{ glm::vec3{ 0.0f, -0.5f, 0.0f } },
        Vertex{ glm::vec3{ 0.5f, 0.5f, 0.0f } }
    };

    std::vector<uint8_t> triangle_indices{
        0, 1, 2
    };

    auto& mesh = m_Meshes.emplace_back(triangle_vertices, triangle_indices);

    mesh.vao.bind();

    mesh.vbo.bind();

    constexpr GLsizei stride = sizeof(Vertex);

    mesh.vao.LinkAttrib(mesh.vbo, 0, 3, GL_FLOAT, stride, (void*) offsetof(Vertex, position));

    mesh.ebo.bind();

    mesh.vbo.unbind();
    mesh.vao.unbind();
    mesh.ebo.unbind();

    return m_Meshes.size() - 1;
}
