/*===============================================

    Forr Engine

    File : VertexBuffers.cpp
    Role : Vertex buffers OpenGL. VAO, VBO, EBO

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VertexBuffers.hpp"

fe::VBO::~VBO() {
    glDeleteBuffers(1, &m_index);
}

void fe::VBO::Create(std::vector<Vertex>& vertices) {
    glGenBuffers(1, &m_index);
    glBindBuffer(GL_ARRAY_BUFFER, m_index);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

void fe::VBO::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_index);
}

void fe::VBO::Unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void fe::VAO::Create() {
    glGenVertexArrays(1, &m_index);
}

void fe::VAO::LinkAttrib(VBO& vbo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset) {
    vbo.Bind();
    glVertexAttribPointer(layout, num_components, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO::Unbind();
}

void fe::VAO::Bind() const {
    glBindVertexArray(m_index);
}

void fe::VAO::Unbind() {
    glBindVertexArray(0);
}

fe::VAO::~VAO() {
    glDeleteVertexArrays(1, &m_index);
}

void fe::EBO::Create(std::vector<GLuint>& indices) {
    glGenBuffers(1, &m_index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void fe::EBO::Create(Indices& indices_variant) {
    glGenBuffers(1, &m_index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index);

    /*std::visit([&](const auto& indices) {
        using T = std::ranges::range_value_t<std::decay_t<decltype(indices)>>;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(T), indices.data(), GL_STATIC_DRAW);
    },
               indices_variant);*/
}

void fe::EBO::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index);
}

void fe::EBO::Unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

fe::EBO::~EBO() {
    glDeleteBuffers(1, &m_index);
}