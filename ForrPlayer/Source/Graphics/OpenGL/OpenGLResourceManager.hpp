/*===============================================

    Forr Engine

    File : OpenGLResourceManager.hpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "Graphics/Model.hpp"

#include "tiny_gltf.h" // temp

#include "OpenGLTypes.hpp"

namespace fe {
    //struct Mesh {
    //    VAO vao;
    //    VBO vbo;
    //    EBO ebo;

    //    uint32_t index_count = 0; // temp

    //    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) : vbo(vertices), ebo(indices), index_count(static_cast<uint32_t>(indices.size())) {}
    //    Mesh(const std::vector<Vertex>& vertices, const Indices& indices_variant) : vbo(vertices), ebo(indices_variant) {
    //        std::visit([&](const auto& indices) {
    //            index_count = static_cast<uint32_t>(indices.size());
    //        },
    //                   indices_variant);
    //    }
    //    ~Mesh() = default;
    //};

    class OpenGLResourceManager {
    public:
        OpenGLResourceManager(ResourceManager& resource_manager) : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        void CreateTexture(const resource::Texture& texture);
        void CreateModel(const resource::Model& model);

    private: // helper functions
        void createMesh(const Mesh& mesh);
        void createPrimitive(const Primitive& primitive, OpenGLPrimitive& opengl_primitive, Vertices& vertices, Indices& indices);

    private:
        ResourceManager& m_ResourceManager;

        fe::typed_pointer_storage<OpenGLTexture> m_Textures{};
        fe::typed_pointer_storage<OpenGLMesh>    m_Meshes{};
    };
} // namespace fe
