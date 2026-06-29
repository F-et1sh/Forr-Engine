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
        // instead, it has 'std::vector<Mesh>', which has 'GPUHandle<Mesh> gpu_handle' in it
        ///@{
        void CreateResource(resource::Model& model);
        void CreateResource(resource::Material& material);
        void CreateResource(resource::Texture& texture);
        ///@}

        const OpenGLMesh&     GetResource(GPUHandle<resource::Model::Mesh> handle) const;
        const OpenGLMaterial& GetResource(GPUHandle<resource::Material> handle) const;
        const OpenGLTexture&  GetResource(GPUHandle<resource::Texture> handle) const;

    private: // here functions, which used like helpers to create some resources that don't have thier own CPU realization.
             // The functions return 'GPUHandle<>' but you DON'T have to set 'GPUHandle<> gpu_handle' in the resources, the functions does it by themselves

        fe::GPUHandle<fe::resource::Model::Mesh> createMesh(resource::Model::Mesh& mesh);

    private: // helpers
        GLuint createShaderProgramRaw(resource::ShaderProgram& shader_program);

    private:
        // this function returns the index of the resource ( GPUHandle<>::index )
        // you DON'T have to set 'GPUHandle<> gpu_handle' in the resource manually, the function does it by itself
        template <typename CPU_T, typename GPU_T>
        size_t storeResource(GPUHandle<CPU_T>& gpu_handle_dst, GPU_T& gpu_resource, std::vector<GPU_T>& storage) {
            storage.emplace_back(std::move(gpu_resource));
            gpu_handle_dst.index = storage.size() - 1;
            return gpu_handle_dst.index;
        }

    private:
        ResourceManager& m_ResourceManager;

        std::vector<OpenGLMaterial> m_StorageMaterials{};
        std::vector<OpenGLMesh>     m_StorageMeshes{};
        std::vector<OpenGLTexture>  m_StorageTextures{};
    };
} // namespace fe
