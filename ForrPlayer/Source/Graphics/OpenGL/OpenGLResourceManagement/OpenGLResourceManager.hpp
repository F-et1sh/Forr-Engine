/*===============================================

    Forr Engine

    File : OpenGLResourceManager.hpp
    Role : GPU Resource Manager ( for OpenGL )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "Graphics/OpenGL/OpenGLTypes.hpp"

namespace fe {
    class OpenGLResourceManager {
    public:
        OpenGLResourceManager(ResourceManager& resource_manager)
            : m_ResourceManager(resource_manager) {}
        ~OpenGLResourceManager() = default;

        // this function won't return you 'GPUHandle<>'
        // you passing 'T&', which is not 'const' -> it sets 'GPUHandle<>' of the resource inside
        // Why : for example, 'fe::resource::Model' does not have 'GPUHandle<Model> gpu_handle' in it
        // instead, it has 'std::vector<Mesh>', which has 'GPUHandle<Mesh> gpu_handle' in it]
        template <resource::resource_t T>
        void CreateResource(T& resource);

        template <typename T>
        const typename GPUResourceTraits<T>::type& GetResource(GPUHandle<T> handle) const;

    private: // here functions, which used like helpers to create some resources that don't have CPU own realization.
             // The functions return 'GPUHandle<>' but you DON'T have to use it. They set 'GPUHandle<> gpu_handle' it the resources by themselves

        fe::GPUHandle<fe::resource::Model::Mesh> createMesh(resource::Model::Mesh& mesh);
        GPUHandle<OpenGLShaderProgram>           createShaderProgram(OpenGLMaterial& opengl_material, std::vector<resource::Shader*> shaders);

    private:
        ResourceManager& m_ResourceManager;

        std::vector<OpenGLMaterial>      m_StorageMaterials{};
        std::vector<OpenGLShaderProgram> m_StorageShaderPrograms{};
        std::vector<OpenGLMesh>          m_StorageMeshes{};
    };
} // namespace fe
