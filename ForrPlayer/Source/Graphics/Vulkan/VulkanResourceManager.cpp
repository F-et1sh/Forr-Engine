/*===============================================

    Forr Engine

    File : VulkanResourceManager.cpp
    Role : GPU Resource Manager ( for Vulkan )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanResourceManager.hpp"

#include "Graphics/Vulkan/VKTools.hpp"

using namespace fe::resource;

void fe::VulkanResourceManager::CreateResource(Material& material) {
    VulkanMaterial vulkan_material{};

    VkDescriptorSetLayout descriptor_set_layout_raw{};
    descriptor_set_layout_raw = this->createDescriptorSetLayout(material);
    vulkan_material.descriptor_set_layout.attach(m_Context.device, descriptor_set_layout_raw);

    VkPipelineLayout pipeline_layout_raw{};
    pipeline_layout_raw = this->createPipelineLayout({ m_Context.global_descriptor_set_layout });
    vulkan_material.pipeline_layout.attach(m_Context.device, pipeline_layout_raw);

    VkPipeline pipeline_raw{};
    pipeline_raw = this->createPipeline(pipeline_layout_raw, material);
    vulkan_material.pipeline.attach(m_Context.device, pipeline_raw);

    // TODO : material.buffer

    this->storeResource(material.gpu_handle, vulkan_material, m_StorageMaterials);
}

void fe::VulkanResourceManager::CreateResource(Model& model) {
    for (auto& mesh : model.meshes) {
        this->createMesh(mesh);
    }
}

void fe::VulkanResourceManager::CreateResource(Texture& texture) {
    VulkanTexture vulkan_texture{};

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    texture.width  = texture.width;
    texture.height = texture.height;
    //texture.mip_levels           = ktxTexture->numLevels;

    VkBool32 use_staging = true;

    bool force_linear_tiling = false;
    if (force_linear_tiling) {
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties(m_Context.physical_device, format, &format_properties);
        use_staging = !(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    }

    VkMemoryAllocateInfo memory_allocate_info = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memory_requirements{};

    if (use_staging) {
        vk::Buffer       staging_buffer{};
        vk::DeviceMemory staging_memory{};

        VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
        bufferCreateInfo.size               = texture.size;
        // This buffer is used as a transfer source for the buffer copy
        bufferCreateInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &bufferCreateInfo, nullptr, &staging_buffer));

        // Get memory requirements for the staging buffer (alignment, memory type bits)
        vkGetBufferMemoryRequirements(m_Context.device, staging_buffer, &memory_requirements);
        memory_allocate_info.allocationSize = memory_requirements.size;
        // Get memory type index for a host visible buffer
        memory_allocate_info.memoryTypeIndex = fe::getMemoryType(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &staging_memory));
        VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, staging_buffer, staging_memory, 0));

        // Copy vulkan_texture data into host local staging buffer
        uint8_t* data;
        VK_CHECK_RESULT(vkMapMemory(m_Context.device, staging_memory, 0, memory_requirements.size, 0, (void**) &data));
        memcpy(data, texture.bytes.get(), ktxTextureSize);
        vkUnmapMemory(m_Context.device, staging_memory);

        // Setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> bufferCopyRegions;
        uint32_t                       offset = 0;

        for (uint32_t i = 0; i < vulkan_texture.mip_levels; i++) {
            // Calculate offset into staging buffer for the current mip level
            ktx_size_t     offset;
            KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
            assert(ret == KTX_SUCCESS);
            // Setup a buffer image copy structure for the current mip level
            VkBufferImageCopy bufferCopyRegion               = {};
            bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel       = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount     = 1;
            bufferCopyRegion.imageExtent.width               = ktxTexture->baseWidth >> i;
            bufferCopyRegion.imageExtent.height              = ktxTexture->baseHeight >> i;
            bufferCopyRegion.imageExtent.depth               = 1;
            bufferCopyRegion.bufferOffset                    = offset;
            bufferCopyRegions.push_back(bufferCopyRegion);
        }

        // Create optimal tiled target image on the m_Context.device
        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
        imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format            = format;
        imageCreateInfo.mipLevels         = vulkan_texture.mip_levels;
        imageCreateInfo.arrayLayers       = 1;
        imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
        // Set initial layout of the image to undefined
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent        = { vulkan_texture.width, vulkan_texture.height, 1 };
        imageCreateInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VK_CHECK_RESULT(vkCreateImage(m_Context.device, &imageCreateInfo, nullptr, &vulkan_texture.image));

        vkGetImageMemoryRequirements(m_Context.device, vulkan_texture.image, &memory_requirements);
        memory_allocate_info.allocationSize  = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = vulkanDevice->getMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &vulkan_texture.deviceMemory));
        VK_CHECK_RESULT(vkBindImageMemory(m_Context.device, vulkan_texture.image, vulkan_texture.deviceMemory, 0));

        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Image memory barriers for the vulkan_texture image

        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = vulkan_texture.mip_levels;
        // The 2D vulkan_texture only has one layer
        subresourceRange.layerCount = 1;

        // Transition the vulkan_texture image layout to transfer target, so we can safely copy our buffer data to it.
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
        ;
        imageMemoryBarrier.image            = vulkan_texture.image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask    = 0;
        imageMemoryBarrier.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
        // Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier);

        // Copy mip levels from staging buffer
        vkCmdCopyBufferToImage(
            copyCmd,
            staging_buffer,
            vulkan_texture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(bufferCopyRegions.size()),
            bufferCopyRegions.data());

        // Once the data has been uploaded we transfer to the vulkan_texture image to the shader read layout, so it can be sampled from
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier);

        // Store current layout for later reuse
        vulkan_texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vulkanDevice->flushCommandBuffer(copyCmd, m_Context.queue_transfer, true);

        // Clean up staging resources
        vkFreeMemory(m_Context.device, staging_memory, nullptr);
        vkDestroyBuffer(m_Context.device, staging_buffer, nullptr);
    }
    else {
        // Copy data to a linear tiled image

        vk::Image        mappable_image{};
        vk::DeviceMemory mappable_memory{};

        // Load mip map level 0 to linear tiling image
        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
        imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format            = format;
        imageCreateInfo.mipLevels         = 1;
        imageCreateInfo.arrayLayers       = 1;
        imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling            = VK_IMAGE_TILING_LINEAR;
        imageCreateInfo.usage             = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageCreateInfo.extent            = { vulkan_texture.width, vulkan_texture.height, 1 };
        VK_CHECK_RESULT(vkCreateImage(m_Context.device, &imageCreateInfo, nullptr, &mappable_image));

        // Get memory requirements for this image like size and alignment
        vkGetImageMemoryRequirements(m_Context.device, mappable_image, &memory_requirements);
        // Set memory allocation size to required memory size
        memory_allocate_info.allocationSize = memory_requirements.size;
        // Get memory type that can be mapped to host memory
        memory_allocate_info.memoryTypeIndex = vulkanDevice->getMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &mappable_memory));
        VK_CHECK_RESULT(vkBindImageMemory(m_Context.device, mappable_image, mappable_memory, 0));

        // Map image memory
        void* data;
        VK_CHECK_RESULT(vkMapMemory(m_Context.device, mappable_memory, 0, memory_requirements.size, 0, &data));
        // Copy image data of the first mip level into memory
        memcpy(data, ktxTextureData, memory_requirements.size);
        vkUnmapMemory(m_Context.device, mappable_memory);

        // Linear tiled images don't need to be staged and can be directly used as textures
        vulkan_texture.image         = std::move(mappable_image);
        vulkan_texture.device_memory = std::move(mappable_memory);
        vulkan_texture.image_layout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Setup image memory barrier transfer image to shader read layout
        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // The sub resource range describes the regions of the image we will be transition
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel            = 0;
        subresourceRange.levelCount              = 1;
        subresourceRange.layerCount              = 1;

        // Transition the vulkan_texture image layout to shader read, so it can be sampled from
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
        ;
        imageMemoryBarrier.image            = vulkan_texture.image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask    = VK_ACCESS_HOST_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout        = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageMemoryBarrier.newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        vkCmdPipelineBarrier(
            copyCmd,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier);

        vulkanDevice->flushCommandBuffer(copyCmd, m_Context.queue_transfer, true);
    }

    ktxTexture_Destroy(ktxTexture);

    // Create a vulkan_texture sampler
    // In Vulkan textures are accessed by samplers
    // This separates all the sampling information from the vulkan_texture data. This means you could have multiple sampler objects for the same vulkan_texture with different settings
    // Note: Similar to the samplers available with OpenGL 3.3
    VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
    sampler.magFilter           = VK_FILTER_LINEAR;
    sampler.minFilter           = VK_FILTER_LINEAR;
    sampler.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.mipLodBias          = 0.0f;
    sampler.compareOp           = VK_COMPARE_OP_NEVER;
    sampler.minLod              = 0.0f;
    // Set max level-of-detail to mip level count of the vulkan_texture
    sampler.maxLod = (use_staging) ? (float) vulkan_texture.mip_levels : 0.0f;
    // Enable anisotropic filtering
    // This feature is optional, so we must check if it's supported on the m_Context.device
    if (m_Context.physical_device_features.samplerAnisotropy) {
        // Use max. level of anisotropy for this example
        sampler.maxAnisotropy    = m_Context.physical_device_properties.limits.maxSamplerAnisotropy;
        sampler.anisotropyEnable = VK_TRUE;
    }
    else {
        // The m_Context.device does not support anisotropic filtering
        sampler.maxAnisotropy    = 1.0;
        sampler.anisotropyEnable = VK_FALSE;
    }
    sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(m_Context.device, &sampler, nullptr, &vulkan_texture.sampler));

    // Create image view
    // Textures are not directly accessed by the shaders and
    // are abstracted by image views containing additional
    // information and sub resource ranges
    VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
    view.viewType              = VK_IMAGE_VIEW_TYPE_2D;
    view.format                = format;
    // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
    // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
    view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel   = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount     = 1;
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    view.subresourceRange.levelCount = (use_staging) ? vulkan_texture.mip_levels : 1;
    // The view will be based on the vulkan_texture's image
    view.image = vulkan_texture.image;
    VK_CHECK_RESULT(vkCreateImageView(m_Context.device, &view, nullptr, &vulkan_texture.view));

    this->storeResource(texture.gpu_handle, vulkan_texture, m_StorageTextures);
}

// TODO : provide fallbacks
#define GET_RESOURCE_INSTANCE(RETURN_T, HANDLE_T, STORAGE)                                     \
    const RETURN_T& fe::VulkanResourceManager::GetResource(GPUHandle<HANDLE_T> handle) const { \
        if (STORAGE.size() <= handle.index) {                                                  \
            fe::logging::fatal("Out of range");                                                \
        }                                                                                      \
        return STORAGE[handle.index];                                                          \
    }

GET_RESOURCE_INSTANCE(fe::VulkanMaterial, fe::resource::Material, m_StorageMaterials)
GET_RESOURCE_INSTANCE(fe::VulkanMesh, fe::resource::Model::Mesh, m_StorageMeshes)
GET_RESOURCE_INSTANCE(fe::VulkanTexture, fe::resource::Texture, m_StorageTextures)

#undef GET_RESOURCE_INSTANCE

fe::GPUHandle<Model::Mesh> fe::VulkanResourceManager::createMesh(resource::Model::Mesh& mesh) {
    VulkanMesh vulkan_mesh{};

    constexpr static VkDeviceSize     offset = 0;
    constexpr static VkMemoryMapFlags flags  = 0;

    struct StagingBuffer { // TODO : this shouldn't be here
        VkDeviceMemory memory{};
        VkBuffer       buffer{};
    };

    struct {
        StagingBuffer vertices;
        StagingBuffer indices;
    } staging_buffers{};

    void* data{};

    /// vertex buffer

    size_t vertex_buffer_size = mesh.vertices.size() * sizeof(Vertex);

    VkBufferCreateInfo vertex_buffer_create_info{};
    vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_create_info.size  = vertex_buffer_size;
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &vertex_buffer_create_info, nullptr, &staging_buffers.vertices.buffer));

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(m_Context.device, staging_buffers.vertices.buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = fe::getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &staging_buffers.vertices.memory));

    VK_CHECK_RESULT(vkMapMemory(m_Context.device, staging_buffers.vertices.memory, offset, memory_allocate_info.allocationSize, flags, &data));
    memcpy(data, mesh.vertices.data(), vertex_buffer_size);
    vkUnmapMemory(m_Context.device, staging_buffers.vertices.memory);

    VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, staging_buffers.vertices.buffer, staging_buffers.vertices.memory, offset));

    ///

    // reusing buffer create info
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBuffer vertex_buffer_raw{};
    VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &vertex_buffer_create_info, nullptr, &vertex_buffer_raw));
    vulkan_mesh.vertex_buffer.buffer.attach(m_Context.device, vertex_buffer_raw); // vertex buffer

    vkGetBufferMemoryRequirements(m_Context.device, vertex_buffer_raw, &memory_requirements); // reusing memory requirements

    // reusing memory allocate info
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = fe::getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory vertex_memory_raw{};
    VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &vertex_memory_raw));
    vulkan_mesh.vertex_buffer.memory.attach(m_Context.device, vertex_memory_raw);

    VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, vertex_buffer_raw, vertex_memory_raw, offset));

    /// index buffer

    vulkan_mesh.index_buffer.count = mesh.indices.size();
    size_t index_buffer_size       = vulkan_mesh.index_buffer.count * sizeof(uint32_t);

    VkBufferCreateInfo index_buffer_create_info{};
    index_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    index_buffer_create_info.size  = index_buffer_size;
    index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &index_buffer_create_info, nullptr, &staging_buffers.indices.buffer));

    vkGetBufferMemoryRequirements(m_Context.device, staging_buffers.indices.buffer, &memory_requirements);

    // reusing memory allocate info
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &staging_buffers.indices.memory));
    VK_CHECK_RESULT(vkMapMemory(m_Context.device, staging_buffers.indices.memory, offset, index_buffer_size, flags, &data));

    memcpy(data, mesh.indices.data(), index_buffer_size);
    vkUnmapMemory(m_Context.device, staging_buffers.indices.memory);
    VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, staging_buffers.indices.buffer, staging_buffers.indices.memory, offset));

    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBuffer index_buffer_raw{};
    VK_CHECK_RESULT(vkCreateBuffer(m_Context.device, &index_buffer_create_info, nullptr, &index_buffer_raw));
    vulkan_mesh.index_buffer.buffer.attach(m_Context.device, index_buffer_raw);

    vkGetBufferMemoryRequirements(m_Context.device, index_buffer_raw, &memory_requirements);

    // reusing memory allocate info
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory index_memory_raw{};
    VK_CHECK_RESULT(vkAllocateMemory(m_Context.device, &memory_allocate_info, nullptr, &index_memory_raw));
    vulkan_mesh.index_buffer.memory.attach(m_Context.device, index_memory_raw);

    VK_CHECK_RESULT(vkBindBufferMemory(m_Context.device, index_buffer_raw, index_memory_raw, offset));

    /// submit

    // there is no RAII because it is going to be freed by freeing fe::RendererVulkan::m_CommandPool
    VkCommandBuffer copy_command_buffer{};

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool        = m_Context.command_pool;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Context.device, &command_buffer_allocate_info, &copy_command_buffer));

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK_RESULT(vkBeginCommandBuffer(copy_command_buffer, &command_buffer_begin_info));

    VkBufferCopy copy_region{};

    // Vertex buffer
    copy_region.size = vertex_buffer_size;
    vkCmdCopyBuffer(copy_command_buffer, staging_buffers.vertices.buffer, vertex_buffer_raw, 1, &copy_region);

    // Index buffer
    copy_region.size = index_buffer_size;
    vkCmdCopyBuffer(copy_command_buffer, staging_buffers.indices.buffer, index_buffer_raw, 1, &copy_region);

    VK_CHECK_RESULT(vkEndCommandBuffer(copy_command_buffer));

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &copy_command_buffer;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = 0;

    fe::vk::Fence fence{}; // for RAII

    VkFence fence_raw{};
    VK_CHECK_RESULT(vkCreateFence(m_Context.device, &fence_create_info, nullptr, &fence_raw));
    fence.attach(m_Context.device, fence_raw);

    VK_CHECK_RESULT(vkQueueSubmit(m_Context.queue_transfer, 1, &submit_info, fence_raw));
    VK_CHECK_RESULT(vkWaitForFences(m_Context.device, 1, &fence_raw, VK_TRUE, m_Context.default_fence_timeout));

    vkFreeCommandBuffers(m_Context.device, m_Context.command_pool, 1, &copy_command_buffer);

    ///

    vkDestroyBuffer(m_Context.device, staging_buffers.vertices.buffer, nullptr);
    vkFreeMemory(m_Context.device, staging_buffers.vertices.memory, nullptr);

    vkDestroyBuffer(m_Context.device, staging_buffers.indices.buffer, nullptr);
    vkFreeMemory(m_Context.device, staging_buffers.indices.memory, nullptr);

    return GPUHandle<Model::Mesh>(this->storeResource(mesh.gpu_handle, vulkan_mesh, m_StorageMeshes));
}

VkDescriptorSetLayout fe::VulkanResourceManager::createDescriptorSetLayout(const Material& material) {
    // NOTE : why only vertex shader ?
    const auto& vertex_shader = *m_ResourceManager.GetResource(material.vertex_shader_ptr);

    std::vector<VkDescriptorSetLayoutBinding> bindings{};
    std::vector<VkDescriptorBindingFlags>     binding_flags{};

    //const auto& reflected_set = vertex_shader.descriptor_sets[1]; // material's descriptor set
    //
    //bindings.reserve(reflected_set.bindings.size());
    //binding_flags.reserve(reflected_set.bindings.size());

    //for (std::size_t i = 0; i < reflected_set.bindings.size(); i++) {
    //    const auto& reflected_binding = reflected_set.bindings[i];

    //    auto& this_binding           = bindings.emplace_back();
    //    this_binding.binding         = reflected_binding.index;
    //    this_binding.descriptorCount = reflected_binding.count;
    //    this_binding.descriptorType  = this->toVkDescriptorType(reflected_binding.descriptor_type);
    //    this_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    //    auto& flags = binding_flags.emplace_back();

    //    if (reflected_binding.is_array) {
    //        flags |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    //        flags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

    //        if (i == reflected_set.bindings.size() - 1) {
    //            flags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    //        }
    //    }
    //    else
    //        flags |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    //}

    VkDescriptorSetLayoutBindingFlagsCreateInfo descriptor_set_layout_binding_flags_create_info{};
    descriptor_set_layout_binding_flags_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    descriptor_set_layout_binding_flags_create_info.bindingCount  = binding_flags.size();
    descriptor_set_layout_binding_flags_create_info.pBindingFlags = binding_flags.data();

    VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info{};
    descriptor_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_create_info.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    descriptor_layout_create_info.bindingCount = bindings.size();
    descriptor_layout_create_info.pBindings    = bindings.data();
    descriptor_layout_create_info.pNext        = &descriptor_set_layout_binding_flags_create_info;

    VkDescriptorSetLayout descriptor_set_layout_raw{};
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_Context.device, &descriptor_layout_create_info, nullptr, &descriptor_set_layout_raw));

    return descriptor_set_layout_raw;
}

VkPipelineLayout fe::VulkanResourceManager::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts_raw) {
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = descriptor_set_layouts_raw.size();
    pipeline_layout_create_info.pSetLayouts    = descriptor_set_layouts_raw.data();

    VkPushConstantRange push_constant{};
    push_constant.offset     = 0;
    push_constant.size       = sizeof(uint32_t); // index
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    pipeline_layout_create_info.pPushConstantRanges    = &push_constant;
    pipeline_layout_create_info.pushConstantRangeCount = 1;

    VkPipelineLayout pipeline_layout_raw{};
    VK_CHECK_RESULT(vkCreatePipelineLayout(m_Context.device, &pipeline_layout_create_info, nullptr, &pipeline_layout_raw));
    return pipeline_layout_raw;
}

VkPipeline fe::VulkanResourceManager::createPipeline(VkPipelineLayout pipeline_layout_raw, const resource::Material& material) {
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.layout     = pipeline_layout_raw;
    graphics_pipeline_create_info.renderPass = m_Context.render_pass;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
    input_assembly_state_create_info.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.cullMode                = VK_CULL_MODE_NONE;
    rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_create_info.depthClampEnable        = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
    rasterization_state_create_info.lineWidth               = 1.0f;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
    color_blend_attachment_state.colorWriteMask = 0xf;
    color_blend_attachment_state.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments    = &color_blend_attachment_state;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.scissorCount  = 1;

    std::vector<VkDynamicState> dynamic_state_enables{};
    dynamic_state_enables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamic_state_enables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
    dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.pDynamicStates    = dynamic_state_enables.data();
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
    depth_stencil_state_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_create_info.depthTestEnable       = VK_TRUE;
    depth_stencil_state_create_info.depthWriteEnable      = VK_TRUE;
    depth_stencil_state_create_info.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_create_info.back.failOp           = VK_STENCIL_OP_KEEP;
    depth_stencil_state_create_info.back.passOp           = VK_STENCIL_OP_KEEP;
    depth_stencil_state_create_info.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    depth_stencil_state_create_info.stencilTestEnable     = VK_FALSE;
    depth_stencil_state_create_info.front                 = depth_stencil_state_create_info.back;

    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding   = 0;
    vertex_input_binding_description.stride    = sizeof(Vertex);
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> vertex_input_attributs{};
    vertex_input_attributs[0].location = 0;
    vertex_input_attributs[0].binding  = 0;
    vertex_input_attributs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributs[0].offset   = offsetof(Vertex, position);

    vertex_input_attributs[1].location = 1;
    vertex_input_attributs[1].binding  = 0;
    vertex_input_attributs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributs[1].offset   = offsetof(Vertex, normal);

    vertex_input_attributs[2].location = 2;
    vertex_input_attributs[2].binding  = 0;
    vertex_input_attributs[2].format   = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attributs[2].offset   = offsetof(Vertex, texture_coord);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
    vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_input_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attributs.size();
    vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attributs.data();

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages_create_info{};

    // vertex shader
    fe::vk::ShaderModule vertex_shader_module = this->createShaderModule(material.vertex_shader_ptr);

    shader_stages_create_info[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages_create_info[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages_create_info[0].module = vertex_shader_module;
    shader_stages_create_info[0].pName  = "main";

    assert(shader_stages_create_info[0].module != VK_NULL_HANDLE);

    // fragment shader
    fe::vk::ShaderModule fragment_shader_module = this->createShaderModule(material.fragment_shader_ptr);

    shader_stages_create_info[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages_create_info[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages_create_info[1].module = fragment_shader_module;
    shader_stages_create_info[1].pName  = "main";

    assert(shader_stages_create_info[1].module != VK_NULL_HANDLE);

    graphics_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages_create_info.size());
    graphics_pipeline_create_info.pStages    = shader_stages_create_info.data();

    graphics_pipeline_create_info.pVertexInputState   = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    graphics_pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
    graphics_pipeline_create_info.pMultisampleState   = &multisample_state_create_info;
    graphics_pipeline_create_info.pViewportState      = &viewport_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState  = &depth_stencil_state_create_info;
    graphics_pipeline_create_info.pDynamicState       = &dynamic_state_create_info;

    VkPipeline pipeline_raw{};
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_Context.device, nullptr, 1, &graphics_pipeline_create_info, nullptr, &pipeline_raw));
    return pipeline_raw;
}

fe::vk::ShaderModule fe::VulkanResourceManager::createShaderModule(fe::pointer<fe::resource::Shader> shader_ptr) {
    // shader's source code must be std::vector<uint32_t>
    static_assert(std::is_same_v<decltype(fe::resource::Shader::source_code), std::vector<uint32_t>>);

    auto& shader = *m_ResourceManager.GetResource(shader_ptr);

    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = shader.source_code.size() * sizeof(uint32_t);
    shader_module_create_info.pCode    = (uint32_t*) shader.source_code.data();

    fe::vk::ShaderModule shader_module{};

    VkShaderModule shader_module_raw{};
    VK_CHECK_RESULT(vkCreateShaderModule(m_Context.device, &shader_module_create_info, nullptr, &shader_module_raw));
    shader_module.attach(m_Context.device, shader_module_raw);

    return std::move(shader_module);
}

VkDescriptorType fe::VulkanResourceManager::toVkDescriptorType(Shader::DescriptorType descriptor_type) const {
    // clang-format off
    switch (descriptor_type) {
        case Shader::DescriptorType::SAMPLER               : return VK_DESCRIPTOR_TYPE_SAMPLER               ; break;
        case Shader::DescriptorType::COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
        case Shader::DescriptorType::SAMPLED_IMAGE         : return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE         ; break;
        case Shader::DescriptorType::STORAGE_IMAGE         : return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         ; break;
        case Shader::DescriptorType::UNIFORM_TEXEL_BUFFER  : return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER  ; break;
        case Shader::DescriptorType::STORAGE_TEXEL_BUFFER  : return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER  ; break;
        case Shader::DescriptorType::UNIFORM_BUFFER        : return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER        ; break;
        case Shader::DescriptorType::STORAGE_BUFFER        : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        ; break;
        case Shader::DescriptorType::UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; break;
        case Shader::DescriptorType::STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC; break;
        case Shader::DescriptorType::INPUT_ATTACHMENT      : return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT      ; break;
        default:
            assert(false);
    }
    // clang-format on

    return VK_DESCRIPTOR_TYPE_SAMPLER;
}
