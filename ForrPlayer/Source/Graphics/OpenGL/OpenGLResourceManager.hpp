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

        // here used 'typename T' instead of 'resource::resource_t T' because this function can be called by GPU types too
        // for example : 'typename T = OpenGLShaderProgram', which is called by 'OpenGLMaterial'
        template <typename T>
        const typename GPUResourceTraits<T>::type& GetResource(GPUHandle<T> handle) const;

    private: // here functions, which used like helpers to create some resources that don't have thier own CPU realization.
             // The functions return 'GPUHandle<>' but you DON'T have to set 'GPUHandle<> gpu_handle' in the resources, the functions does it by themselves

        fe::GPUHandle<fe::resource::Model::Mesh> createMesh(resource::Model::Mesh& mesh);
        GPUHandle<OpenGLShaderProgram>           createShaderProgram(OpenGLMaterial& opengl_material, std::vector<resource::Shader*> shaders);

    private:
        // this function returns the index of the resource ( GPUHandle<>::index )
        // you DON'T have to set 'GPUHandle<> gpu_handle' in the resources by yourself, the function does it by itself
        template <typename T, typename GPU_T>
        size_t storeResource(GPUHandle<T>& gpu_handle_dst, GPU_T& gpu_resource, std::vector<GPU_T>& storage) {
            storage.emplace_back(std::move(gpu_resource));
            gpu_handle_dst.index = storage.size() - 1;
            return gpu_handle_dst.index;
        }

    private:
        ResourceManager& m_ResourceManager;

        std::vector<OpenGLMaterial>      m_StorageMaterials{};
        std::vector<OpenGLShaderProgram> m_StorageShaderPrograms{};
        std::vector<OpenGLMesh>          m_StorageMeshes{};
        std::vector<OpenGLTexture>       m_StorageTextures{};
    };
} // namespace fe
