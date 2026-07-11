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

    vulkan_texture.width  = texture.width;
    vulkan_texture.height = texture.height;

    bool generate_mips        = texture.mip_levels.empty();
    vulkan_texture.mip_levels = generate_mips
                                    ? static_cast<uint32_t>(std::log2(std::max(vulkan_texture.width, vulkan_texture.height)) + 1)
                                    : static_cast<uint32_t>(texture.mip_levels.size());

    VkBool32 use_staging = true;

    bool force_linear_tiling = false;
    if (force_linear_tiling) {
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties(m_Context.physical_device, format, &format_properties);
        use_staging = ((format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0);
    }

    if (use_staging) {
        vk::VmaBuffer staging_buffer = fe::createBuffer(m_Context, texture.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        VmaAllocationInfo allocation_info{};
        vmaGetAllocationInfo(m_Context.allocator, staging_buffer.get<VmaAllocation>(), &allocation_info);
        std::memcpy(allocation_info.pMappedData, texture.bytes.get(), texture.size);

        vulkan_texture.image = fe::createImage(m_Context,
                                               VK_IMAGE_TYPE_2D,
                                               format,
                                               { vulkan_texture.width, vulkan_texture.height, 1 },
                                               vulkan_texture.mip_levels,
                                               1,
                                               VK_SAMPLE_COUNT_1_BIT,
                                               VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_LAYOUT_UNDEFINED,
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        std::vector<VkBufferImageCopy> buffer_copy_regions{};

        if (texture.mip_levels.empty()) {
            VkBufferImageCopy buffer_copy_region{};
            buffer_copy_region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            buffer_copy_region.imageSubresource.mipLevel       = 0;
            buffer_copy_region.imageSubresource.baseArrayLayer = 0;
            buffer_copy_region.imageSubresource.layerCount     = 1;
            buffer_copy_region.imageExtent.width               = texture.width;
            buffer_copy_region.imageExtent.height              = texture.height;
            buffer_copy_region.imageExtent.depth               = 1;
            buffer_copy_region.bufferOffset                    = 0;

            buffer_copy_regions.emplace_back(buffer_copy_region);
        }
        else {
            buffer_copy_regions.reserve(texture.mip_levels.size());
            for (uint32_t i = 0; i < texture.mip_levels.size(); i++) {
                VkBufferImageCopy buffer_copy_region{};
                buffer_copy_region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_copy_region.imageSubresource.mipLevel       = i;
                buffer_copy_region.imageSubresource.baseArrayLayer = 0;
                buffer_copy_region.imageSubresource.layerCount     = 1;
                buffer_copy_region.imageExtent.width               = texture.mip_levels[i].width;
                buffer_copy_region.imageExtent.height              = texture.mip_levels[i].height;
                buffer_copy_region.imageExtent.depth               = 1;
                buffer_copy_region.bufferOffset                    = texture.mip_levels[i].offset;

                buffer_copy_regions.emplace_back(buffer_copy_region);
            }
        }

        fe::runOneTimeCommands(m_Context, [&](VkCommandBuffer command_buffer) {
            VkImageSubresourceRange subresource_range{};
            subresource_range.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
            subresource_range.baseMipLevel = 0;
            subresource_range.levelCount   = vulkan_texture.mip_levels;
            subresource_range.layerCount   = 1;

            VkImageMemoryBarrier barrier{};
            barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image               = vulkan_texture.image.get<VkImage>();
            barrier.subresourceRange    = subresource_range;
            barrier.srcAccessMask       = VkAccessFlags{};
            barrier.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags{}, 0, nullptr, 0, nullptr, 1, &barrier);

            vkCmdCopyBufferToImage(command_buffer, staging_buffer.get<VkBuffer>(), vulkan_texture.image.get<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(buffer_copy_regions.size()), buffer_copy_regions.data());

            if (generate_mips) {
                this->generateMipmaps(command_buffer,
                                      vulkan_texture.image.get<VkImage>(),
                                      vulkan_texture.width,
                                      vulkan_texture.height,
                                      vulkan_texture.mip_levels);
            }
            else {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags{}, 0, nullptr, 0, nullptr, 1, &barrier);
            }
        });

        vulkan_texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else {
        vulkan_texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vulkan_texture.image        = fe::createImage(m_Context,
                                                      VK_IMAGE_TYPE_2D,
                                                      format,
                                                      { vulkan_texture.width, vulkan_texture.height, 1 },
                                                      1,
                                                      1,
                                                      VK_SAMPLE_COUNT_1_BIT,
                                                      VK_IMAGE_TILING_LINEAR,
                                                      VK_IMAGE_LAYOUT_PREINITIALIZED,
                                                      VK_IMAGE_USAGE_SAMPLED_BIT);
    }

    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.maxAnisotropy = 1.0f;
    sampler_create_info.magFilter     = VK_FILTER_LINEAR;
    sampler_create_info.minFilter     = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.mipLodBias    = 0.0f;
    sampler_create_info.compareOp     = VK_COMPARE_OP_NEVER;
    sampler_create_info.minLod        = 0.0f;
    sampler_create_info.maxLod        = (use_staging) ? (float) vulkan_texture.mip_levels : 0.0f;

    // TODO : provide anisotropy sampling though the settings instead using max level
    if (m_Context.physical_device_features.samplerAnisotropy) {
        sampler_create_info.maxAnisotropy    = m_Context.physical_device_properties.limits.maxSamplerAnisotropy;
        sampler_create_info.anisotropyEnable = VK_TRUE;
    }
    else {
        sampler_create_info.maxAnisotropy    = 1.0;
        sampler_create_info.anisotropyEnable = VK_FALSE;
    }
    sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    VkSampler sampler_raw{};
    VK_CHECK_RESULT(vkCreateSampler(m_Context.device, &sampler_create_info, nullptr, &sampler_raw));
    vulkan_texture.sampler.attach(m_Context.device, sampler_raw);

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format                          = format;
    image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel   = 0;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount     = 1;
    image_view_create_info.subresourceRange.levelCount     = (use_staging) ? vulkan_texture.mip_levels : 1;
    image_view_create_info.image                           = vulkan_texture.image.get<VkImage>();

    VkImageView image_view_raw{};
    VK_CHECK_RESULT(vkCreateImageView(m_Context.device, &image_view_create_info, nullptr, &image_view_raw));
    vulkan_texture.image_view.attach(m_Context.device, image_view_raw);

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

    vulkan_mesh.vertex_buffer = fe::createDeviceLocalBuffer(m_Context, mesh.vertices.data(), sizeof(Vertex) * mesh.vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vulkan_mesh.index_buffer  = fe::createDeviceLocalBuffer(m_Context, mesh.indices.data(), sizeof(Index) * mesh.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    return GPUHandle<Model::Mesh>(this->storeResource(mesh.gpu_handle, vulkan_mesh, m_StorageMeshes));
}

VkDescriptorSetLayout fe::VulkanResourceManager::createDescriptorSetLayout(const Material& material) {
    // NOTE : why only vertex shader ?
    //const auto& vertex_shader = *m_ResourceManager.GetResource(material.vertex_shader_ptr);

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

    auto& shader_program = *m_ResourceManager.GetResource(material.shader_program_ptr);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_create_infos{};
    std::vector<fe::vk::ShaderModule>            shader_modules_raii{};

    shader_stages_create_infos.reserve(shader_program.source_codes.size());
    shader_modules_raii.reserve(shader_program.source_codes.size());

    for (const auto& [shader_type, source_code] : shader_program.source_codes) {
        auto& shader_stages_create_info = shader_stages_create_infos.emplace_back();

        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = source_code.size();
        shader_module_create_info.pCode    = reinterpret_cast<const uint32_t*>(source_code.data());

        VkShaderModule shader_module_raw{};
        VK_CHECK_RESULT(vkCreateShaderModule(m_Context.device, &shader_module_create_info, nullptr, &shader_module_raw));
        shader_modules_raii.emplace_back().attach(m_Context.device, shader_module_raw);

        shader_stages_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages_create_info.module = shader_module_raw;

        switch (shader_type) {
            case shader::Type::VERTEX:
                shader_stages_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                shader_stages_create_info.pName = "vertexMain";
                break;
            case shader::Type::FRAGMENT:
                shader_stages_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shader_stages_create_info.pName = "fragmentMain";
                break;
            case shader::Type::COMPUTE:
                shader_stages_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                shader_stages_create_info.pName = "computeMain";
                break;
            default:
                fe::logging::error("Unknown shader type : %i", static_cast<uint32_t>(shader_type));

                shader_stages_create_info.stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
                shader_stages_create_info.pName = nullptr;
                break;
        }
    }

    graphics_pipeline_create_info.stageCount          = static_cast<uint32_t>(shader_stages_create_infos.size());
    graphics_pipeline_create_info.pStages             = shader_stages_create_infos.data();
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

void fe::VulkanResourceManager::generateMipmaps(VkCommandBuffer command_buffer, VkImage image_raw, uint32_t width, uint32_t height, uint32_t mip_levels) {
    VkImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.image                           = image_raw;
    image_memory_barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount     = 1;
    image_memory_barrier.subresourceRange.levelCount     = 1;

    int mip_width  = width;
    int mip_height = height;

    for (uint32_t i = 1; i < mip_levels; i++) {
        image_memory_barrier.subresourceRange.baseMipLevel = i - 1;
        image_memory_barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_memory_barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        image_memory_barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags{}, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        VkImageBlit blit{};

        // src
        blit.srcOffsets[0]                 = VkOffset3D{ 0, 0, 0 };
        blit.srcOffsets[1]                 = VkOffset3D{ mip_width, mip_height, 1 };
        blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel       = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount     = 1;

        // dst
        blit.dstOffsets[0]                 = VkOffset3D{ 0, 0, 0 };
        blit.dstOffsets[1]                 = VkOffset3D{ mip_width > 1 ? mip_width / 2 : 1,
                                                         mip_height > 1 ? mip_height / 2 : 1,
                                                         1 };
        blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel       = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount     = 1;

        vkCmdBlitImage(command_buffer, image_raw, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image_raw, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        image_memory_barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        image_memory_barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags{}, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }

    image_memory_barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    image_memory_barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    image_memory_barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_memory_barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_memory_barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VkDependencyFlags{}, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

VkDescriptorType fe::VulkanResourceManager::toVkDescriptorType(/*ShaderProgram::DescriptorType descriptor_type*/) const {
    //// clang-format off
    //switch (descriptor_type) {
    //    case ShaderProgram::DescriptorType::SAMPLER               : return VK_DESCRIPTOR_TYPE_SAMPLER               ; break;
    //    case ShaderProgram::DescriptorType::COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
    //    case ShaderProgram::DescriptorType::SAMPLED_IMAGE         : return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE         ; break;
    //    case ShaderProgram::DescriptorType::STORAGE_IMAGE         : return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE         ; break;
    //    case ShaderProgram::DescriptorType::UNIFORM_TEXEL_BUFFER  : return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER  ; break;
    //    case ShaderProgram::DescriptorType::STORAGE_TEXEL_BUFFER  : return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER  ; break;
    //    case ShaderProgram::DescriptorType::UNIFORM_BUFFER        : return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER        ; break;
    //    case ShaderProgram::DescriptorType::STORAGE_BUFFER        : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER        ; break;
    //    case ShaderProgram::DescriptorType::UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; break;
    //    case ShaderProgram::DescriptorType::STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC; break;
    //    case ShaderProgram::DescriptorType::INPUT_ATTACHMENT      : return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT      ; break;
    //    default:
    //        assert(false);
    //}
    //// clang-format on

    return VK_DESCRIPTOR_TYPE_SAMPLER;
}
