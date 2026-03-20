/*===============================================

    Forr Engine

    File : VulkanResourceCreator.cpp
    Role : creates Vulkan resources from unified ones

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanResourceCreator.hpp"

fe::pointer<fe::VulkanTexture> fe::VulkanResourceCreator::CreateResource(const resource::Texture& texture) {
    VulkanTexture this_texture{};

    auto ptr = m_Storage.m_Textures.create(std::move(this_texture));
    return ptr;
}

FORR_NODISCARD fe::pointer<fe::VulkanModel> fe::VulkanResourceCreator::CreateResource(const resource::Model& model) {
    VulkanModel this_model{};

    for (const auto& mesh : model.meshes) {
        auto ptr = this->createMesh(mesh);
    }

    auto ptr = m_Storage.m_Models.create(std::move(this_model));
    return ptr;
}

fe::pointer<fe::VulkanMesh> fe::VulkanResourceCreator::createMesh(const resource::Model::Mesh& mesh) {
    VulkanMesh vulkan_mesh{};

    // TODO :
    //vulkan_mesh.vertex_buffer -> mesh.vertices
    //vulkan_mesh.index_buffer -> mesh.indices

    for (const auto& primitive : mesh.primitives) {
        VulkanPrimitive& vulkan_primitive = vulkan_mesh.primitives.emplace_back();

        vulkan_primitive.index_count  = primitive.index_count;
        vulkan_primitive.index_offset = primitive.index_offset;
        vulkan_primitive.material_ptr = primitive.material_ptr;
    }

    auto ptr = m_Storage.m_Meshes.create(std::move(vulkan_mesh));
    return ptr;
}
