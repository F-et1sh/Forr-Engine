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
    private:
        template <template <typename> class C>
        struct Wrapper {};

    public:
        OpenGLResourceManager(ResourceManager& resource_manager) : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        void CreateTexture(const resource::Texture& texture);
        void CreateModel(fe::pointer<fe::resource::Model> model_ptr);

    private: // helper functions
        void createMesh(const Mesh& mesh);
        void createPrimitive(const Primitive& primitive, OpenGLPrimitive& opengl_primitive, Vertices& vertices, Indices& indices);

    private:
        ResourceManager& m_ResourceManager;

        std::unordered_map<fe::pointer<...>,
                           fe::pointer<...>>
            m_CPU_GPU_Lookup{};

        fe::typed_pointer_storage<OpenGLModel>   m_Models{};
        fe::typed_pointer_storage<OpenGLTexture> m_Textures{};
        fe::typed_pointer_storage<OpenGLMesh>    m_Meshes{};
    };
} // namespace fe
