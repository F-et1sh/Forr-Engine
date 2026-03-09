/*===============================================

    Forr Engine

    File : OpenGLResourceManager.hpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "VertexBuffers.hpp"
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
        OpenGLResourceManager()  = default;
        ~OpenGLResourceManager() = default;

        MeshID CreateTriangle(); // TODO : this mustn't return MeshID
        void   CreateTexture(const resource::Texture& texture);

        //FORR_FORCE_INLINE FORR_NODISCARD Mesh& getMesh(MeshID index) { return m_Meshes[index]; }

    private:
        void loadMeshes(const tinygltf::Model& model);
        void loadPrimitives(const tinygltf::Model& model, std::vector<Primitive>& this_primitives, const std::vector<tinygltf::Primitive>& primitives);
        void loadVertices(const tinygltf::Model& model, Vertices& this_vertices, Indices& this_indices, const tinygltf::Primitive& primitive);
        void loadIndices(const tinygltf::Model& model, Primitive& this_primitive, Indices& this_indices, const tinygltf::Primitive& primitive);

        template <typename T>
        [[nodiscard]] bool fastCopy(const tinygltf::Accessor& accessor, const tinygltf::BufferView& buffer_view, const tinygltf::Buffer& buffer, std::vector<T>& out) {
            if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                size_t element_size = sizeof(T);
                size_t stride       = buffer_view.byteStride ? buffer_view.byteStride : element_size;
                if (stride == element_size) {
                    const uint8_t* src = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
                    out.resize(accessor.count);
                    memcpy(out.data(), src, accessor.count * element_size);
                    return true;
                }
            }
            return false;
        }

        template <typename T>
            requires(std::is_same_v<T, glm::vec2> ||
                     std::is_same_v<T, glm::vec3> ||
                     std::is_same_v<T, glm::vec4> ||
                     std::is_same_v<T, glm::uvec4> ||
                     std::is_same_v<T, glm::u16vec4> ||
                     std::is_same_v<T, glm::mat4>)
        void readAttribute(const tinygltf::Model& model, size_t accessor_index, std::vector<T>& out) {
            const tinygltf::Accessor&   accessor    = model.accessors[accessor_index];
            const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer&     buffer      = model.buffers[buffer_view.buffer];

            const uint8_t* data_ptr       = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
            int            component_size = tinygltf::GetComponentSizeInBytes(accessor.componentType);
            int            num_components = tinygltf::GetNumComponentsInType(accessor.type);

            auto   element_size = static_cast<size_t>(component_size * num_components);
            size_t stride       = buffer_view.byteStride != 0 ? buffer_view.byteStride : element_size;

            out.resize(accessor.count);

            if (fastCopy<T>(accessor, buffer_view, buffer, out)) {
                return;
            }

            for (size_t i = 0; i < accessor.count; i++) {
                const uint8_t* p = data_ptr + (i * stride);

                if constexpr (std::is_same_v<T, glm::vec2>) {
                    const auto* f = reinterpret_cast<const float*>(p);
                    out[i]        = glm::vec2(f[0], f[1]);
                }
                else if constexpr (std::is_same_v<T, glm::vec3>) {
                    const auto* f = reinterpret_cast<const float*>(p);
                    out[i]        = glm::vec3(f[0], f[1], f[2]);
                }
                else if constexpr (std::is_same_v<T, glm::vec4>) {
                    const auto* f = reinterpret_cast<const float*>(p);
                    out[i]        = glm::vec4(f[0], f[1], f[2], f[3]);
                }
                else if constexpr (std::is_same_v<T, glm::u16vec4>) {
                    const auto* ptr = reinterpret_cast<const uint16_t*>(p);
                    out[i]          = glm::u16vec4(ptr[0], ptr[1], ptr[2], ptr[3]);
                }
                else if constexpr (std::is_same_v<T, glm::mat4>) {
                    glm::mat4 m{};
                    memcpy(glm::value_ptr(m), data_ptr + (i * stride), sizeof(glm::mat4));
                    out[i] = m;
                }
            }
        }

    private:
        fe::typed_pointer_storage<OpenGLTexture> m_Textures{};
        fe::typed_pointer_storage<OpenGLMesh> m_Meshes{};
    };
} // namespace fe
