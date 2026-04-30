/*===============================================

    Forr Engine

    File : VulkanResourceManager.cpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanResourceManager.hpp"

using namespace fe::resource;

template <>
void fe::VulkanResourceManager::CreateResource(Material& material) {
    
}
template void fe::VulkanResourceManager::CreateResource(Material& material);

///

template <>
void fe::VulkanResourceManager::CreateResource(Model& model) {
    for (auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }
}
template void fe::VulkanResourceManager::CreateResource(Model& model);

///

template <>
void fe::VulkanResourceManager::CreateResource(Texture& texture) {
    
}
template void fe::VulkanResourceManager::CreateResource(Texture& texture);

///

//template<>
//const fe::VulkanMaterial& fe::VulkanResourceManager::GetResource(GPUHandle<resource::Material> handle) const {
//    return m_StorageMaterials[handle.index];
//}
//template const fe::VulkanMaterial& fe::VulkanResourceManager::GetResource(GPUHandle<resource::Material> handle)const;
//
//template<>
//const fe::VulkanShaderProgram& fe::VulkanResourceManager::GetResource(GPUHandle<VulkanShaderProgram> handle) const {
//    return m_StorageShaderPrograms[handle.index];
//}
//template const fe::VulkanShaderProgram& fe::VulkanResourceManager::GetResource(GPUHandle<VulkanShaderProgram> handle)const;

template<>
const fe::VulkanMesh& fe::VulkanResourceManager::GetResource(GPUHandle<resource::Model::Mesh> handle) const {
    return m_StorageMeshes[handle.index];
}
template const fe::VulkanMesh& fe::VulkanResourceManager::GetResource(GPUHandle<resource::Model::Mesh> handle)const;


///

fe::GPUHandle<Model::Mesh> fe::VulkanResourceManager::createMesh(resource::Model::Mesh& mesh) {
    VulkanMesh Vulkan_mesh{};

    

    return GPUHandle<Model::Mesh>(this->storeResource(mesh.gpu_handle, Vulkan_mesh, m_StorageMeshes));
}

//fe::GPUHandle<fe::VulkanShaderProgram> fe::VulkanResourceManager::createShaderProgram(VulkanMaterial& Vulkan_material, std::vector<resource::Shader*> shaders) {
//
//}