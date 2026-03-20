/*===============================================

    Forr Engine

    File : VulkanResourceManager.cpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanResourceManager.hpp"

#include "Graphics/GPUTypes.hpp"
#include "Graphics/Vulkan/VKTools.hpp"

void fe::VulkanResourceManager::CreateTexture(fe::pointer<fe::resource::Texture> texture_ptr) {
    const auto& texture = *m_ResourceManager.GetResource(texture_ptr);

    auto ptr = m_Importer.ImportTexture(texture); // does not need to store this fe::pointer


}

void fe::VulkanResourceManager::CreateModel(fe::pointer<fe::resource::Model> cpu_model_ptr) { // TODO : the texture or model might be already created, handle it
    VulkanModel vulkan_model{};

    const auto& model = *m_ResourceManager.GetResource(cpu_model_ptr);

    // mesh hasn't its own extension, so you get a structure here, not a pointer
    for (const auto& mesh : model.meshes) {
        auto ptr = this->createMesh(mesh);
        vulkan_model.pointers_mesh.emplace_back(ptr);

        for (const auto& primitive : mesh.primitives) {
            auto material = m_ResourceManager.GetResource(primitive.material_ptr);

            if (!material) continue; // TODO : provide fallbacks

            auto texture_ptr = material->pbr_metallic_roughness.base_color_texture.texture_ptr; // TODO : at first review all this shit, because I just can't read and/or edit this. After that, provide materials

            auto ptr = this->createTexture(texture_ptr);
            vulkan_model.pointers_texture.emplace_back(ptr);
        }
    }

    auto gpu_ptr = m_Models.create(std::move(vulkan_model));
    m_CPU_GPU_Model.insert({ cpu_model_ptr.packed(), gpu_ptr.packed() });
}

fe::pointer<fe::VulkanMesh> fe::VulkanResourceManager::createMesh(const Mesh& mesh) {
    VulkanMesh vulkan_mesh{};

    for (const auto& primitive : mesh.primitives) {

        VulkanPrimitive vulkan_primitive{};

        this->createPrimitive(primitive, vulkan_primitive);

        vulkan_mesh.primitives.emplace_back(std::move(vulkan_primitive));
    }

    auto ptr = m_Meshes.create(std::move(vulkan_mesh));
    return ptr;
}

fe::pointer<fe::VulkanTexture> fe::VulkanResourceManager::createTexture(fe::pointer<fe::resource::Texture> cpu_texture_ptr) {

    const auto& texture = *m_ResourceManager.GetResource(cpu_texture_ptr);

    VulkanTexture vulkan_texture{};
    // TODO : create vulkan texture

    auto gpu_ptr = m_Textures.create(std::move(vulkan_texture));
    m_CPU_GPU_Texture.insert({ cpu_texture_ptr.packed(), gpu_ptr.packed() });

    return gpu_ptr;
}

void fe::VulkanResourceManager::createPrimitive(const Primitive& primitive, VulkanPrimitive& vulkan_primitive) {
    //constexpr static VkDeviceSize     offset = 0;
    //constexpr static VkMemoryMapFlags flags  = 0;

    //struct StagingBuffer { // TODO : this mustn't be here
    //    VkDeviceMemory memory{};
    //    VkBuffer       buffer{};
    //};

    //struct {
    //    StagingBuffer vertices;
    //    StagingBuffer indices;
    //} staging_buffers{};

    //void* data{};

    ///// vertex buffer

    //size_t vertex_buffer_size = primitive.vertices.size() * sizeof(Vertex);

    //VkBufferCreateInfo vertex_buffer_create_info{};
    //vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //vertex_buffer_create_info.size  = vertex_buffer_size;
    //vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    //VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &vertex_buffer_create_info, nullptr, &staging_buffers.vertices.buffer));

    //VkMemoryRequirements memory_requirements{};
    //vkGetBufferMemoryRequirements(m_Context.device, staging_buffers.vertices.buffer, &memory_requirements);

    //VkMemoryAllocateInfo memory_allocate_info{};
    //memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //memory_allocate_info.allocationSize  = memory_requirements.size;
    //memory_allocate_info.memoryTypeIndex = fe::getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &staging_buffers.vertices.memory));

    //VK_CHECK_RESULT(vkMapMemory(m_Context.device, staging_buffers.vertices.memory, offset, memory_allocate_info.allocationSize, flags, &data));
    //memcpy(data, primitive.vertices.data(), vertex_buffer_size);
    //vkUnmapMemory(m_Context.device, staging_buffers.vertices.memory);

    //VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, staging_buffers.vertices.buffer, staging_buffers.vertices.memory, offset));

    /////

    //// reusing buffer create info
    //vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    //VkBuffer vertex_buffer_raw{};
    //VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &vertex_buffer_create_info, nullptr, &vertex_buffer_raw));
    //vulkan_primitive.vertex_buffer.buffer.attach(m_Context.device, vertex_buffer_raw); // vertex buffer

    //vkGetBufferMemoryRequirements(m_Context.device, vertex_buffer_raw, &memory_requirements); // reusing memory requirements

    //// reusing memory allocate info
    //memory_allocate_info.allocationSize  = memory_requirements.size;
    //memory_allocate_info.memoryTypeIndex = fe::getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //VkDeviceMemory vertex_memory_raw{};
    //VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &vertex_memory_raw));
    //vulkan_primitive.vertex_buffer.memory.attach(m_Context.device, vertex_memory_raw);

    //VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, vertex_buffer_raw, vertex_memory_raw, offset));

    ///// index buffer

    //vulkan_primitive.index_buffer.count = primitive.indices.size();
    //size_t index_buffer_size            = vulkan_primitive.index_buffer.count * sizeof(uint32_t);

    //VkBufferCreateInfo indexbuffer_create_info{};
    //indexbuffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //indexbuffer_create_info.size  = index_buffer_size;
    //indexbuffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    //VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &indexbuffer_create_info, nullptr, &staging_buffers.indices.buffer));

    //vkGetBufferMemoryRequirements(m_Context.device, staging_buffers.indices.buffer, &memory_requirements);

    //// reusing memory allocate info
    //memory_allocate_info.allocationSize  = memory_requirements.size;
    //memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &staging_buffers.indices.memory));
    //VK_CHECK_RESULT(vkMapMemory(m_Context.device, staging_buffers.indices.memory, offset, index_buffer_size, flags, &data));

    //memcpy(data, primitive.indices.data(), index_buffer_size);
    //vkUnmapMemory(m_Context.device, staging_buffers.indices.memory);
    //VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, staging_buffers.indices.buffer, staging_buffers.indices.memory, offset));

    //indexbuffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    //VkBuffer index_buffer_raw{};
    //VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &indexbuffer_create_info, nullptr, &index_buffer_raw));
    //vulkan_primitive.index_buffer.buffer.attach(m_Context.device, index_buffer_raw);

    //vkGetBufferMemoryRequirements(m_Context.device, index_buffer_raw, &memory_requirements);

    //// reusing memory allocate info
    //memory_allocate_info.allocationSize  = memory_requirements.size;
    //memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //VkDeviceMemory index_memory_raw{};
    //VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &index_memory_raw));
    //vulkan_primitive.index_buffer.memory.attach(m_Context.device, index_memory_raw);

    //VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, index_buffer_raw, index_memory_raw, offset));

    ///// submit

    //// there is no RAII because it is going to be freed by freeing m_CommandPool
    //VkCommandBuffer copy_command_buffer{};

    //VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    //command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    //command_buffer_allocate_info.commandPool        = m_Context.command_pool;
    //command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //command_buffer_allocate_info.commandBufferCount = 1;
    //VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Context.device, &command_buffer_allocate_info, &copy_command_buffer));

    //VkCommandBufferBeginInfo command_buffer_begin_info{};
    //command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //VK_CHECK_RESULT(vkBeginCommandBuffer(copy_command_buffer, &command_buffer_begin_info));

    //VkBufferCopy copy_region{};

    //// Vertex buffer
    //copy_region.size = vertex_buffer_size;
    //vkCmdCopyBuffer(copy_command_buffer, staging_buffers.vertices.buffer, vertex_buffer_raw, 1, &copy_region);

    //// Index buffer
    //copy_region.size = index_buffer_size;
    //vkCmdCopyBuffer(copy_command_buffer, staging_buffers.indices.buffer, index_buffer_raw, 1, &copy_region);

    //VK_CHECK_RESULT(vkEndCommandBuffer(copy_command_buffer));

    //VkSubmitInfo submit_info{};
    //submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //submit_info.commandBufferCount = 1;
    //submit_info.pCommandBuffers    = &copy_command_buffer;

    //VkFenceCreateInfo fence_create_info{};
    //fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //fence_create_info.flags = 0;

    //fe::vk::Fence fence{}; // for RAII

    //VkFence fence_raw{};
    //VK_CHECK_RESULT(vkCreateFence(m_Context.device, &fence_create_info, nullptr, &fence_raw));
    //fence.attach(m_Context.device, fence_raw);

    //VK_CHECK_RESULT(vkQueueSubmit(m_Context.queue_transfer, 1, &submit_info, fence_raw));
    //VK_CHECK_RESULT(vkWaitForFences(m_Context.device, 1, &fence_raw, VK_TRUE, m_Context.default_fence_timeout));

    //vkFreeCommandBuffers(m_Context.device, m_Context.command_pool, 1, &copy_command_buffer);

    /////

    //vkDestroyBuffer(m_Context.device, staging_buffers.vertices.buffer, nullptr);
    //vkFreeMemory(m_Context.device, staging_buffers.vertices.memory, nullptr);

    //vkDestroyBuffer(m_Context.device, staging_buffers.indices.buffer, nullptr);
    //vkFreeMemory(m_Context.device, staging_buffers.indices.memory, nullptr);

    //vulkan_primitive.index_count  = primitive.index_count;
    //vulkan_primitive.index_offset = primitive.index_offset;
}
