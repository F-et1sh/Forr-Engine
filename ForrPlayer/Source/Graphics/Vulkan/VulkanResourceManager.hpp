/*===============================================

    Forr Engine

    File : VulkanResourceManager.hpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ResourceManagement/ResourceManager.hpp"
#include "VulkanTypes.hpp"
#include "VulkanContext.hpp"

namespace fe {
    class VulkanResourceManager {
    public:
        VulkanResourceManager(VulkanContext& context, ResourceManager& resource_manager)
            : m_Context(context), m_ResourceManager(resource_manager) {}
        ~VulkanResourceManager() = default;

        // this function won't return you 'GPUHandle<>'
        // you passing 'T&', which is not 'const' -> it sets 'GPUHandle<>' of the resource inside
        // Why : for example, 'fe::resource::Model' does not have 'GPUHandle<Model> gpu_handle' in it
        // instead, it has 'std::vector<Mesh>', which has 'GPUHandle<Mesh> gpu_handle' in it]
        template <resource::resource_t T>
        void CreateResource(T& resource);

        // here used 'typename T' instead of 'resource::resource_t T' because this function can be called by GPU types too
        template <typename T>
        const typename VulkanResourceTraits<T>::type& GetResource(GPUHandle<T> handle) const;

    private: // here functions, which used like helpers to create some resources that don't have thier own CPU realization.
             // The functions return 'GPUHandle<>' but you DON'T have to set 'GPUHandle<> gpu_handle' in the resources, the functions does it by themselves

        fe::GPUHandle<fe::resource::Model::Mesh> createMesh(resource::Model::Mesh& mesh);
        //GPUHandle<VulkanShaderProgram>           createShaderProgram(VulkanMaterial& Vulkan_material, std::vector<resource::Shader*> shaders);

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
        VulkanContext&   m_Context;
        ResourceManager& m_ResourceManager;

        //std::vector<VulkanMaterial>      m_StorageMaterials{};
        //std::vector<VulkanShaderProgram> m_StorageShaderPrograms{};
        std::vector<VulkanMesh>    m_StorageMeshes{};
        std::vector<VulkanTexture> m_StorageTextures{};
    };
} // namespace fe
