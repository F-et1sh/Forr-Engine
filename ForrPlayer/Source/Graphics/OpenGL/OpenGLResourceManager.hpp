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
    class OpenGLResourceManager {
    public:
        OpenGLResourceManager(ResourceManager& resource_manager) : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        // TODO : make a separate function for this
        fe::pointer<OpenGLTexture> CreateTexture(fe::pointer<fe::resource::Texture> cpu_texture_ptr);
        void CreateModel(fe::pointer<fe::resource::Model> cpu_model_ptr);

        template <typename GPU, typename CPU>
        const GPU* GetResource(fe::pointer<CPU> cpu_ptr) {
            const auto& lookup_table = this->GetLookupTable<CPU, GPU>();

            auto it = lookup_table.find(cpu_ptr.packed());
            assert(it != lookup_table.end());

            const auto& storage = this->GetStorage<GPU>();
            
            fe::pointer<GPU> gpu_ptr{};
            gpu_ptr.unpack(it->second);

            assert(storage.is_valid(gpu_ptr));

            return storage.get(gpu_ptr);
        }

        // unsafe helper function
        template <typename T>
        fe::typed_pointer_storage<T>& GetStorage() {
            if constexpr (std::is_same_v<T, OpenGLTexture>) {
                return m_Textures;
            }
            else if constexpr (std::is_same_v<T, OpenGLModel>) {
                return m_Models;
            }
            else if constexpr (std::is_same_v<T, OpenGLMesh>) {
                return m_Meshes;
            }
            else {
                assert(false);
            }
        }

        template <typename CPU, typename GPU>
        const std::unordered_map<uint64_t, uint64_t>& GetLookupTable() const {
            if constexpr (std::is_same_v<CPU, fe::resource::Texture> &&
                          std::is_same_v<GPU, OpenGLTexture>) {
                return m_CPU_GPU_Texture;
            }
            else if constexpr (std::is_same_v<CPU, fe::resource::Model> &&
                               std::is_same_v<GPU, OpenGLModel>) {
                return m_CPU_GPU_Model;
            }
            else {
                assert(false);
            }
        }

    private: // helper functions
        fe::pointer<OpenGLMesh> createMesh(const Mesh& mesh);
        void                    createPrimitive(const Primitive& primitive, OpenGLPrimitive& opengl_primitive);

    private:
        ResourceManager& m_ResourceManager;

        std::unordered_map<uint64_t, uint64_t> m_CPU_GPU_Texture{};
        std::unordered_map<uint64_t, uint64_t> m_CPU_GPU_Model{};

        fe::typed_pointer_storage<OpenGLTexture> m_Textures{};
        fe::typed_pointer_storage<OpenGLModel>   m_Models{};
        fe::typed_pointer_storage<OpenGLMesh>    m_Meshes{};
    };
} // namespace fe
