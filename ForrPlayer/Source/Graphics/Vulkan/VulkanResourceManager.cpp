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

template <>
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

    this->storeResource(material.gpu_handle, vulkan_material, m_StorageMaterials);
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

// TODO : provide fallbacks
#define GET_RESOURCE_INSTANCE(RETURN_T, HANDLE_T, STORAGE)                                     \
    template <>                                                                                \
    const RETURN_T& fe::VulkanResourceManager::GetResource(GPUHandle<HANDLE_T> handle) const { \
        if (STORAGE.size() <= handle.index) {                                                  \
            fe::logging::fatal("Out of range");                                                \
        }                                                                                      \
        return STORAGE[handle.index];                                                          \
    }                                                                                          \
    template const RETURN_T& fe::VulkanResourceManager::GetResource(GPUHandle<HANDLE_T> handle) const;

GET_RESOURCE_INSTANCE(fe::VulkanMaterial, fe::resource::Material, m_StorageMaterials)
GET_RESOURCE_INSTANCE(fe::VulkanMesh, fe::resource::Model::Mesh, m_StorageMeshes)
GET_RESOURCE_INSTANCE(fe::VulkanTexture, fe::resource::Texture, m_StorageTextures)

#undef GET_RESOURCE_INSTANCE
///

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
    std::vector<VkDescriptorBindingFlags> binding_flags{};

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
