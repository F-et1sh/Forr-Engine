/*===============================================

    Forr Engine

    File : GPUResourceManager.hpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "VertexBuffers.hpp"

namespace fe {
    struct Mesh {
        VAO vao;
        VBO vbo;
        EBO ebo;

        uint32_t index_count = 0; // temp

        Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) : vbo(vertices), ebo(indices), index_count(static_cast<uint32_t>(indices.size())) {}
        Mesh(const std::vector<Vertex>& vertices, const Indices& indices_variant) : vbo(vertices), ebo(indices_variant) {
            std::visit([&](const auto& indices) {
                index_count = static_cast<uint32_t>(indices.size());
            },
                       indices_variant);
        }
        ~Mesh() = default;
    };

    class GPUResourceManager {
    public:
        GPUResourceManager()  = default;
        ~GPUResourceManager() = default;

        MeshID                                 CreateTriangle();
        FORR_FORCE_INLINE FORR_NODISCARD Mesh& getMesh(MeshID index) { return m_Meshes[index]; }

    private:
        std::vector<Mesh> m_Meshes;
    };
} // namespace fe
