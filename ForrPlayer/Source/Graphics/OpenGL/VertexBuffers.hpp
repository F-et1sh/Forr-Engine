/*===============================================

    Forr Engine

    File : VertexBuffers.hpp
    Role : Vertex buffers OpenGL. VAO, VBO, EBO

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "GPUTypes.hpp"

#include <vector>
#include <variant>
#include <glad/gl.h>

namespace fe {
    using Vertices = std::vector<Vertex>;
    using Indices  = std::variant<
         std::vector<uint8_t>,
         std::vector<uint16_t>,
         std::vector<uint32_t>>;

    class VBO {
    public:
        VBO(const std::vector<Vertex>& vertices);
        ~VBO();

        void bind() const;
        void unbind();

        FORR_FORCE_INLINE FORR_NODISCARD unsigned int index() const noexcept { return m_Index; }

    private:
        GLuint m_Index;
    };

    class VAO {
    public:
        VAO();
        ~VAO();

        void LinkAttrib(VBO& vbo, GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void* offset);

        void bind() const;
        void unbind();

        FORR_FORCE_INLINE FORR_NODISCARD unsigned int index() const noexcept { return m_Index; }

    private:
        GLuint m_Index;
    };

    class EBO {
    public:
        EBO(const std::vector<GLuint>& indices);
        EBO(const Indices& indices_variant);
        ~EBO();

        void bind() const;
        void unbind();

        FORR_FORCE_INLINE FORR_NODISCARD unsigned int index() const noexcept { return m_Index; }

    private:
        GLuint m_Index;
    };
} // namespace fe
