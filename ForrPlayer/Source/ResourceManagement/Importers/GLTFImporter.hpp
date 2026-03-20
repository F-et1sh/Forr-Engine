/*===============================================

    Forr Engine

    File : GLTFImporter.hpp
    Role : imports resources and their metadata. for gltf and glb

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceStorage.hpp"
#include "ResourceManagement/Resources.hpp"

#include "tiny_gltf.h"

namespace fe {
    struct GLTFImportContext {
    public:
        const tinygltf::Model& model;
        resource::Model&       this_model;
        ResourceStorage&       storage;

        std::vector<fe::pointer<resource::Texture>>  textures;
        std::vector<fe::pointer<resource::Material>> materials;

        // safe function
        fe::pointer<resource::Texture> GetTexture(uint32_t index)const noexcept {
            if (index >= textures.size()) return {}; // TODO : provide fallbacks
            return textures[index];
        }

        // safe function
        fe::pointer<resource::Material> GetMaterial(uint32_t index)const noexcept {
            if (index >= materials.size()) return {}; // TODO : provide fallbacks
            return materials[index];
        }

        GLTFImportContext()  = default;
        ~GLTFImportContext() = default;

        GLTFImportContext(const tinygltf::Model& model, resource::Model& this_model, ResourceStorage& storage)
            : model(model), this_model(this_model), storage(storage) {}
    };

    class GLTFImporter {
    private:
        inline static constexpr size_t JOINTS_COUNT = 128; // temp
    public:
        GLTFImporter()  = default;
        ~GLTFImporter() = default;

        static void Import(ResourceStorage& storage, const std::filesystem::path& resource_full_path);

    private:
        static void loadNodes(GLTFImportContext& context);
        static void loadSceneRoots(GLTFImportContext& context);
        static void loadSkins(GLTFImportContext& context);
        static void loadMeshes(GLTFImportContext& context);
        static void loadTextures(GLTFImportContext& context);
        static void loadMaterials(GLTFImportContext& context);
        static void loadPrimitives(GLTFImportContext& context, std::vector<resource::Model::Primitive>& this_primitives, const std::vector<tinygltf::Primitive>& primitives);
        static void loadVertices(GLTFImportContext& context, Vertices& this_vertices, Indices& this_indices, const tinygltf::Primitive& primitive);
        static void loadIndices(GLTFImportContext& context, resource::Model::Primitive& this_primitive, Indices& this_indices, const tinygltf::Primitive& primitive);
        static void loadAnimations(GLTFImportContext& context);

    private:
        static fe::pointer<resource::Texture>  createTexture(const tinygltf::Model& model, uint32_t texture_index, ResourceStorage& storage);
        static fe::pointer<resource::Material> createMaterial(GLTFImportContext& context, uint32_t tinygltf_material_index);

    private:
        template <typename T>
            requires(std::is_same_v<T, glm::vec2> ||
                     std::is_same_v<T, glm::vec3> ||
                     std::is_same_v<T, glm::vec4> ||
                     std::is_same_v<T, glm::uvec4> ||
                     std::is_same_v<T, glm::u16vec4> ||
                     std::is_same_v<T, glm::mat4>)
        static void readAttribute(const tinygltf::Model& model, size_t accessor_index, std::vector<T>& out) {
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

        template <typename T>
        static FORR_NODISCARD bool fastCopy(const tinygltf::Accessor& accessor, const tinygltf::BufferView& buffer_view, const tinygltf::Buffer& buffer, std::vector<T>& out) {
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

        static void readVector(glm::vec2& dst, const std::vector<double>& src);
        static void readVector(glm::vec3& dst, const std::vector<double>& src);
        static void readVector(glm::vec4& dst, const std::vector<double>& src);
        static void readVector(glm::quat& dst, const std::vector<double>& src);

        static FORR_NODISCARD float readComponentAsFloat(const uint8_t* data, int component_type, bool normalized);

        static void readAccessorVec4(const tinygltf::Model& model, int accessor_index, std::vector<glm::vec4>& out);
        static void readAccessorFloat(const tinygltf::Model& model, int accessor_index, std::vector<float>& out);
    };

} // namespace fe
