/*===============================================

    Forr Engine

    File : VertexBuffers.hpp
    Role : Vertex buffers OpenGL. VAO, VBO, EBO.
        This is going to be removed

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "Graphics/GPUTypes.hpp"

#include <vector>
#include <variant>
#include <glad/gl.h>

namespace fe {
    class VBO {
    public:
        VBO() = default;
        ~VBO();

        void Create(std::vector<Vertex>& vertices);

        void        Bind() const;
        static void Unbind();

    private:
        GLuint m_index;
    };

    class VAO {
    public:
        VAO() = default;
        ~VAO();

        void Create();

        static void LinkAttrib(VBO& vbo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset);

        void        Bind() const;
        static void Unbind();

    private:
        GLuint m_index;
    };

    class EBO {
    public:
        EBO() = default;
        ~EBO();

        void Create(std::vector<GLuint>& indices);
        void Create(Indices& indices_variant);

        void        Bind() const;
        static void Unbind();

        [[nodiscard]] unsigned int index() const noexcept { return m_index; }

    private:
        GLuint m_index;
    };
} // namespace fe
