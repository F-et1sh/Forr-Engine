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
    glDeleteBuffers(1, &m_Index);
}

fe::VBO::VBO(const std::vector<Vertex>& vertices) {
    glGenBuffers(1, &m_Index);
    glBindBuffer(GL_ARRAY_BUFFER, m_Index);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

void fe::VBO::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_Index);
}

void fe::VBO::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

fe::VAO::VAO() {
    glGenVertexArrays(1, &m_Index);
}

void fe::VAO::LinkAttrib(VBO& vbo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset) {
    vbo.bind();
    glVertexAttribPointer(layout, num_components, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    vbo.unbind();
}

void fe::VAO::bind() const {
    glBindVertexArray(m_Index);
}

void fe::VAO::unbind() {
    glBindVertexArray(0);
}

fe::VAO::~VAO() {
    glDeleteVertexArrays(1, &m_Index);
}

fe::EBO::EBO(const std::vector<GLuint>& indices) {
    glGenBuffers(1, &m_Index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

fe::EBO::EBO(const Indices& indices_variant) {
    glGenBuffers(1, &m_Index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Index);

    std::visit([&](const auto& indices) {
        using T = std::ranges::range_value_t<std::decay_t<decltype(indices)>>;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(T), indices.data(), GL_STATIC_DRAW);
    },
               indices_variant);
}

void fe::EBO::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Index);
}

void fe::EBO::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

fe::EBO::~EBO() {
    glDeleteBuffers(1, &m_Index);
}
