/*===============================================

    Forr Engine

    File : RendererVulkn.cpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "RendererVulkan.hpp"

#include <fstream>
#include <unordered_set>

#include "Tools.hpp"

fe::RendererVulkan::RendererVulkan(const RendererDesc& desc,
                                   IPlatformSystem&    platform_system,
                                   size_t              primary_window_index)
    : m_Description(desc),
      m_PlatformSystem(platform_system),
      m_PrimaryWindow(m_PlatformSystem.getWindow(primary_window_index)) {

    this->configureCamera();

    this->InitializeBase();
    this->InitializeDevice();
    this->InitializeSwapchain();
    this->InitializeCommandBuffers();
    this->InitializeSynchronizationPrimitives();
    this->InitializeDepthStencil();
    this->InitializeRenderPass();
    this->InitializePipelineCache();
    this->InitializeFramebuffers();
    this->InitializeVertexBuffer();
    this->InitializeUniformBuffer();
    this->InitializeDescriptors();
    this->InitializePipeline();
}

fe::RendererVulkan::~RendererVulkan() {
}

void fe::RendererVulkan::ClearScreen(float red, float green, float blue, float alpha) {
}

void fe::RendererVulkan::SwapBuffers() {
}

void fe::RendererVulkan::Draw(MeshID index) {
    this->VKRender();
}

void fe::RendererVulkan::configureCamera() {
    m_Camera.setType(Camera::Type::LOOKAT);
    m_Camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    m_Camera.setRotation(glm::vec3(0.0f));

    float fov    = 60.0f;
    float aspect = (float) m_PrimaryWindow.getWidth() / (float) m_PrimaryWindow.getHeight();
    float znear  = 1.0f;
    float zfar   = 1000.0f;
    m_Camera.setPerspective(fov, aspect, znear, zfar);
}

void fe::RendererVulkan::resizeWindow() {
    m_IsWindowResized = true;

    vkDeviceWaitIdle(m_Device);

    m_Swapchain.CreateSwapchain();

    this->InitializeDepthStencil();

    this->InitializeFramebuffers();

    this->InitializeSynchronizationPrimitives();

    vkDeviceWaitIdle(m_Device);

    int width  = m_PrimaryWindow.getWidth();
    int height = m_PrimaryWindow.getHeight();

    if ((width > 0.0f) && (height > 0.0f)) {
        m_Camera.updateAspectRatio((float) width / (float) height);
    }
}

void fe::RendererVulkan::InitializeBase() {
    VK_CHECK_RESULT(volkInitialize());

    this->VKCreateInstance();
    this->VKChoosePhysicalDevice();
    this->VKSetupDepthStencilFormat();

    // TODO : After VKChoosePhysicalDevice() you getting
    // m_Context::physical_device_features - they are not enabled.
    // So, it's needed to provide ability to enable some of them if you need
}

void fe::RendererVulkan::InitializeDevice() {

    // TODO : Add enabled features adding

    this->VKSetupQueueFamilyProperties();
    this->VKSetupSupportedExtensions();

    // TODO : Add enabled extensions adding

    this->VKCreateDevice();
    this->VKCreateCommandPool();
    this->VKSetupQueues();
}

void fe::RendererVulkan::InitializeSwapchain() {
    m_Swapchain.CreateSurface();
    m_Swapchain.SetupSurfaceColorFormat();
    m_Swapchain.SetupQueueNodeIndex();
    m_Swapchain.CreateSwapchain();
}

void fe::RendererVulkan::InitializeCommandBuffers() {
    assert(m_CommandPool);

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};

    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool        = m_CommandPool;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

    VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, m_CommandBuffers.data()));
}

void fe::RendererVulkan::InitializeSynchronizationPrimitives() {
    std::array<VkFence, VulkanContext::max_concurrent_frames>     wait_fences_raw{};
    std::array<VkSemaphore, VulkanContext::max_concurrent_frames> present_complete_semaphores_raw{};
    std::vector<VkSemaphore>                                      render_complete_semaphores_raw{};

    ///

    for (size_t i = 0; i < wait_fences_raw.size(); i++) {
        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto& fence = wait_fences_raw[i];
        VK_CHECK_RESULT(vkCreateFence(m_Device, &fence_create_info, nullptr, &fence));

        m_WaitFences[i].attach(m_Device, fence);
    }

    ///

    for (size_t i = 0; i < present_complete_semaphores_raw.size(); i++) {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto& semaphore = present_complete_semaphores_raw[i];
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, nullptr, &semaphore));

        m_PresentCompleteSemaphores[i].attach(m_Device, semaphore);
    }

    ///

    render_complete_semaphores_raw.resize(m_Context.swapchain_image_count);
    m_RenderCompleteSemaphores.resize(m_Context.swapchain_image_count);

    for (size_t i = 0; i < render_complete_semaphores_raw.size(); i++) {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto& semaphore = render_complete_semaphores_raw[i];
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, nullptr, &semaphore));

        m_RenderCompleteSemaphores[i].attach(m_Device, semaphore);
    }
}

void fe::RendererVulkan::InitializeDepthStencil() {
    VkImageCreateInfo image_create_info{};
    image_create_info.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType   = VK_IMAGE_TYPE_2D;
    image_create_info.format      = m_Context.depth_format;
    image_create_info.extent      = { m_Context.swapchain_extent.width, m_Context.swapchain_extent.height, 1 };
    image_create_info.mipLevels   = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples     = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling      = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImage depth_stencil_image_raw{};
    VK_CHECK_RESULT(vkCreateImage(m_Device, &image_create_info, nullptr, &depth_stencil_image_raw));
    m_DepthStencil.image.attach(m_Device, depth_stencil_image_raw);

    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements(m_Device, m_DepthStencil.image, &memory_requirements);

    VkMemoryAllocateInfo memory_alllocate_info{};
    memory_alllocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alllocate_info.allocationSize  = memory_requirements.size;
    memory_alllocate_info.memoryTypeIndex = fe::getMemoryType(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory depth_stencil_memory_raw{};
    VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_alllocate_info, nullptr, &depth_stencil_memory_raw));
    m_DepthStencil.memory.attach(m_Device, depth_stencil_memory_raw);

    VK_CHECK_RESULT(vkBindImageMemory(m_Device, m_DepthStencil.image, m_DepthStencil.memory, 0));

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image            = m_DepthStencil.image;
    image_view_create_info.viewType         = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format           = m_Context.depth_format;
    image_view_create_info.subresourceRange = {
        .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    if (m_Context.depth_format >= VK_FORMAT_D16_UNORM_S8_UINT) {
        image_view_create_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageView depth_stencil_image_view_raw{};
    VK_CHECK_RESULT(vkCreateImageView(m_Device, &image_view_create_info, nullptr, &depth_stencil_image_view_raw));
    m_DepthStencil.image_view.attach(m_Device, depth_stencil_image_view_raw);
}

void fe::RendererVulkan::InitializeRenderPass() {
    // if dynamic rendering enabled there is no need in render pass
    if (m_Context.use_dynamic_rendering) return;

    std::array<VkAttachmentDescription, 2> attachments{
        // color attachment
        VkAttachmentDescription{
            .format         = m_Context.swapchain_color_format,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },

        // depth attachment
        VkAttachmentDescription{
            .format         = m_Context.depth_format,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
    };

    VkAttachmentReference color_reference{ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depth_reference{ .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount    = 1;
    subpass_description.pColorAttachments       = &color_reference;
    subpass_description.pDepthStencilAttachment = &depth_reference;

    std::array<VkSubpassDependency, 2> dependencies{
        VkSubpassDependency{
            .srcSubpass    = VK_SUBPASS_EXTERNAL,
            .dstSubpass    = 0,
            .srcStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        },
        VkSubpassDependency{
            .srcSubpass    = VK_SUBPASS_EXTERNAL,
            .dstSubpass    = 0,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        }
    };

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments    = attachments.data();
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass_description;
    render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    render_pass_info.pDependencies   = dependencies.data();

    VkRenderPass render_pass{};
    VK_CHECK_RESULT(vkCreateRenderPass(m_Device, &render_pass_info, nullptr, &render_pass));
    m_RenderPass.attach(m_Device, render_pass);

    // === SETUP CONTEXT ===
    m_Context.render_pass = render_pass; // render pass
}

void fe::RendererVulkan::InitializePipelineCache() {
    VkPipelineCacheCreateInfo pipeline_cache_create_info{};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    VkPipelineCache pipeline_cache{};
    VK_CHECK_RESULT(vkCreatePipelineCache(m_Device, &pipeline_cache_create_info, nullptr, &pipeline_cache));
    m_PipelineCache.attach(m_Device, pipeline_cache);

    // === SETUP CONTEXT ===
    m_Context.pipeline_cache = pipeline_cache; // pipeline cache
}

void fe::RendererVulkan::InitializeFramebuffers() {
    // if dynamic rendering enabled there is no need in render pass
    if (m_Context.use_dynamic_rendering) return;

    m_Framebuffers.resize(m_Context.swapchain_image_count);
    m_Context.framebuffers.resize(m_Context.swapchain_image_count);

    for (size_t i = 0; i < m_Framebuffers.size(); i++) {
        const auto& swapchain_image_views = m_Swapchain.getImageViews();

        const VkImageView attachments[2] = { swapchain_image_views[i], m_DepthStencil.image_view };

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = m_RenderPass;
        framebuffer_create_info.attachmentCount = 2;
        framebuffer_create_info.pAttachments    = attachments;
        framebuffer_create_info.width           = m_Context.swapchain_extent.width;
        framebuffer_create_info.height          = m_Context.swapchain_extent.height;
        framebuffer_create_info.layers          = 1;

        VkFramebuffer framebuffer{};
        VK_CHECK_RESULT(vkCreateFramebuffer(m_Device, &framebuffer_create_info, nullptr, &framebuffer));
        m_Framebuffers[i].attach(m_Device, framebuffer);

        // === SETUP CONTEXT ===
        m_Context.framebuffers[i] = framebuffer; // framebuffer
    }
}

void fe::RendererVulkan::InitializeVertexBuffer() {

    constexpr static VkDeviceSize     offset = 0;
    constexpr static VkMemoryMapFlags flags  = 0;

    struct StagingBuffer {
        VkDeviceMemory memory{};
        VkBuffer       buffer{};
    };

    struct {
        StagingBuffer vertices;
        StagingBuffer indices;
    } staging_buffers{};

    void* data{};

    /// vertex buffer

    std::vector<Vertex> vertex_buffer{
        { { 1.0f, 1.0f, 0.0f } },
        { { -1.0f, 1.0f, 0.0f } },
        { { 0.0f, -1.0f, 0.0f } }
    };

    size_t vertex_buffer_size = vertex_buffer.size() * sizeof(Vertex);

    VkBufferCreateInfo vertex_buffer_create_info{};
    vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_create_info.size  = vertex_buffer_size;
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VK_CHECK_RESULT(vkCreateBuffer(m_Device, &vertex_buffer_create_info, nullptr, &staging_buffers.vertices.buffer));

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(m_Device, staging_buffers.vertices.buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = fe::getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_allocate_info, nullptr, &staging_buffers.vertices.memory));

    VK_CHECK_RESULT(vkMapMemory(m_Device, staging_buffers.vertices.memory, offset, memory_allocate_info.allocationSize, flags, &data));
    memcpy(data, vertex_buffer.data(), vertex_buffer_size);
    vkUnmapMemory(m_Device, staging_buffers.vertices.memory);

    VK_CHECK_RESULT(vkBindBufferMemory(m_Device, staging_buffers.vertices.buffer, staging_buffers.vertices.memory, offset));

    ///

    // reusing buffer create info
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBuffer vertex_buffer_raw{};
    VK_CHECK_RESULT(vkCreateBuffer(m_Device, &vertex_buffer_create_info, nullptr, &vertex_buffer_raw));
    m_VertexBuffer.buffer.attach(m_Device, vertex_buffer_raw); // vertex buffer

    vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer.buffer, &memory_requirements); // reusing memory requirements

    // reusing memory allocate info
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = fe::getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory vertex_memory_raw{};
    VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_allocate_info, nullptr, &vertex_memory_raw));
    m_VertexBuffer.memory.attach(m_Device, vertex_memory_raw);

    VK_CHECK_RESULT(vkBindBufferMemory(m_Device, vertex_buffer_raw, vertex_memory_raw, offset));

    /// index buffer

    std::vector<uint32_t> index_buffer{ 0, 1, 2 };

    m_IndexBuffer.count      = index_buffer.size();
    size_t index_buffer_size = m_IndexBuffer.count * sizeof(uint32_t);

    VkBufferCreateInfo indexbuffer_create_info{};
    indexbuffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexbuffer_create_info.size  = index_buffer_size;
    indexbuffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VK_CHECK_RESULT(vkCreateBuffer(m_Device, &indexbuffer_create_info, nullptr, &staging_buffers.indices.buffer));

    vkGetBufferMemoryRequirements(m_Device, staging_buffers.indices.buffer, &memory_requirements);

    // reusing memory allocate info
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_allocate_info, nullptr, &staging_buffers.indices.memory));
    VK_CHECK_RESULT(vkMapMemory(m_Device, staging_buffers.indices.memory, offset, index_buffer_size, flags, &data));

    memcpy(data, index_buffer.data(), index_buffer_size);
    vkUnmapMemory(m_Device, staging_buffers.indices.memory);
    VK_CHECK_RESULT(vkBindBufferMemory(m_Device, staging_buffers.indices.buffer, staging_buffers.indices.memory, offset));

    indexbuffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBuffer index_buffer_raw{};
    VK_CHECK_RESULT(vkCreateBuffer(m_Device, &indexbuffer_create_info, nullptr, &index_buffer_raw));
    m_IndexBuffer.buffer.attach(m_Device, index_buffer_raw);

    vkGetBufferMemoryRequirements(m_Device, m_IndexBuffer.buffer, &memory_requirements);

    // reusing memory allocate info
    memory_allocate_info.allocationSize  = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory index_memory_raw{};
    VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_allocate_info, nullptr, &index_memory_raw));
    m_IndexBuffer.memory.attach(m_Device, index_memory_raw);

    VK_CHECK_RESULT(vkBindBufferMemory(m_Device, index_buffer_raw, index_memory_raw, offset));

    /// submit

    // there is no RAII because it is going to be freed by freeing m_CommandPool
    VkCommandBuffer copy_command_buffer{};

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool        = m_CommandPool;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, &copy_command_buffer));

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK_RESULT(vkBeginCommandBuffer(copy_command_buffer, &command_buffer_begin_info));

    VkBufferCopy copy_region{};

    // Vertex buffer
    copy_region.size = vertex_buffer_size;
    vkCmdCopyBuffer(copy_command_buffer, staging_buffers.vertices.buffer, m_VertexBuffer.buffer, 1, &copy_region);

    // Index buffer
    copy_region.size = index_buffer_size;
    vkCmdCopyBuffer(copy_command_buffer, staging_buffers.indices.buffer, m_IndexBuffer.buffer, 1, &copy_region);

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
    VK_CHECK_RESULT(vkCreateFence(m_Device, &fence_create_info, nullptr, &fence_raw));
    fence.attach(m_Device, fence_raw);

    VK_CHECK_RESULT(vkQueueSubmit(m_Context.queue_transfer, 1, &submit_info, fence_raw));
    VK_CHECK_RESULT(vkWaitForFences(m_Device, 1, &fence_raw, VK_TRUE, m_Context.default_fence_timeout));

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &copy_command_buffer);

    ///

    vkDestroyBuffer(m_Device, staging_buffers.vertices.buffer, nullptr);
    vkFreeMemory(m_Device, staging_buffers.vertices.memory, nullptr);

    vkDestroyBuffer(m_Device, staging_buffers.indices.buffer, nullptr);
    vkFreeMemory(m_Device, staging_buffers.indices.memory, nullptr);
}

void fe::RendererVulkan::InitializeUniformBuffer() {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size  = sizeof(ShaderData);
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    for (size_t i = 0; i < VulkanContext::max_concurrent_frames; i++) {

        VkBuffer buffer_raw{};
        VK_CHECK_RESULT(vkCreateBuffer(m_Device, &buffer_create_info, nullptr, &buffer_raw));
        m_UniformBuffers[i].buffer.attach(m_Device, buffer_raw);

        VkMemoryRequirements memory_requirements{};
        vkGetBufferMemoryRequirements(m_Device, buffer_raw, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize  = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDeviceMemory memory_raw{};
        VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_allocate_info, nullptr, &memory_raw));
        m_UniformBuffers[i].memory.attach(m_Device, memory_raw);

        constexpr static VkDeviceSize     offset = 0;
        constexpr static VkMemoryMapFlags flags  = 0;

        VK_CHECK_RESULT(vkBindBufferMemory(m_Device, buffer_raw, memory_raw, offset));
        VK_CHECK_RESULT(vkMapMemory(m_Device, memory_raw, offset, sizeof(ShaderData), flags, (void**) &m_UniformBuffers[i].mapped));
    }
}

void fe::RendererVulkan::InitializeDescriptors() {
    this->VKSetupDescriptorSetLayout();
    this->VKSetupDescriptorPool();
    this->VKSetupDescriptorSets();
}

void fe::RendererVulkan::InitializePipeline() {
    this->VKSetupPipelineLayout();

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.layout     = m_PipelineLayout;
    graphics_pipeline_create_info.renderPass = m_RenderPass;

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

    std::array<VkVertexInputAttributeDescription, 1> vertex_input_attributs{};
    vertex_input_attributs[0].binding  = 0;
    vertex_input_attributs[0].location = 0;
    vertex_input_attributs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_input_attributs[0].offset   = offsetof(Vertex, position);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
    vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
    vertex_input_state_create_info.pVertexBindingDescriptions      = &vertex_input_binding_description;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
    vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attributs.data();

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages_create_info{};

    // vertex shader
    fe::vk::ShaderModule vertex_shader_module = this->createShaderModule(PATH.getShadersPath() / "default.vert.spv");

    shader_stages_create_info[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages_create_info[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages_create_info[0].module = vertex_shader_module;
    shader_stages_create_info[0].pName  = "main";

    assert(shader_stages_create_info[0].module != VK_NULL_HANDLE);

    // fragment shader
    fe::vk::ShaderModule fragment_shader_module = this->createShaderModule(PATH.getShadersPath() / "default.frag.spv");

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
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_Device, m_PipelineCache, 1, &graphics_pipeline_create_info, nullptr, &pipeline_raw));
    m_Pipeline.attach(m_Device, pipeline_raw); // pipeline
}

void fe::RendererVulkan::VKCreateInstance() {
    std::vector<const char*> surface_extensions = this->m_PlatformSystem.getSurfaceRequiredExtensions();

    for (const char* e : surface_extensions)
        m_Context.enabled_instance_extensions.push_back(e);

    uint32_t extension_properties_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_properties_count, nullptr);

    if (extension_properties_count > 0) {

        std::vector<VkExtensionProperties> extension_properties(extension_properties_count);

        VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_properties_count, extension_properties.data());
        if (result == VK_SUCCESS) {
            for (const auto& e : extension_properties) {
                m_Context.supported_instance_extensions.push_back(e.extensionName);
            }
        }
    }

    std::vector<std::string> enabled_instance_extensions_copy = m_Context.enabled_instance_extensions; // copy
    std::vector<size_t>      extensions_to_remove{};

    for (size_t i = 0; i < enabled_instance_extensions_copy.size(); i++) {
        const auto& e = enabled_instance_extensions_copy[i];

        auto it = std::find(m_Context.supported_instance_extensions.begin(),
                            m_Context.supported_instance_extensions.end(),
                            e);

        if (it == m_Context.supported_instance_extensions.end())
            extensions_to_remove.push_back(i);
    }

    for (auto i : extensions_to_remove) {
        auto it = m_Context.enabled_instance_extensions.begin() + i;
        m_Context.enabled_instance_extensions.erase(it);
    }

    if constexpr (true /* m_ShaderDir == "slang" */) {
        if constexpr (m_Context.api_version < VK_API_VERSION_1_1) { // this is hardcoded for now. It will always be false
            //m_Context.api_version = VK_API_VERSION_1_1;
        }

        m_Context.enabled_physical_device_extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        m_Context.enabled_physical_device_extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
        m_Context.enabled_physical_device_extensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
    }

    VkApplicationInfo application_info{};
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    application_info.pApplicationName   = m_Description.application_name.c_str(),
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName        = "Forr Engine";
    application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion         = m_Context.api_version;

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    instance_create_info.pApplicationInfo = &application_info;

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};
    if (m_Description.validation_enabled) {
        debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        debug_utils_messenger_create_info.pfnUserCallback = debugUtilsMessageCallback;

        debug_utils_messenger_create_info.pNext = instance_create_info.pNext;
        instance_create_info.pNext              = &debug_utils_messenger_create_info;
    }

    auto it = std::find(m_Context.supported_instance_extensions.begin(),
                        m_Context.supported_instance_extensions.end(),
                        VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (m_Description.validation_enabled || it != m_Context.supported_instance_extensions.end()) {
        m_Context.enabled_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // must be in this scope
    std::vector<const char*> enabled_instance_extensions_cstr{};
    for (const auto& e : m_Context.enabled_instance_extensions)
        enabled_instance_extensions_cstr.push_back(e.c_str());

    if (!m_Context.enabled_instance_extensions.empty()) {
        instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_instance_extensions_cstr.size());
        instance_create_info.ppEnabledExtensionNames = enabled_instance_extensions_cstr.data();
    }

    const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

    if (m_Description.validation_enabled) {

        uint32_t instance_layer_count{};
        vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);

        std::vector<VkLayerProperties> instance_layer_properties(instance_layer_count);
        vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layer_properties.data());

        bool validation_layer_present = false;
        for (auto& e : instance_layer_properties) {

            if (strcmp(e.layerName, validation_layer_name) == 0) {
                validation_layer_present = true;
                break;
            }
        }
        if (validation_layer_present) {
            instance_create_info.ppEnabledLayerNames = &validation_layer_name;
            instance_create_info.enabledLayerCount   = 1;
        }
        else
            fe::logging::error("Validation layer %s not present, validation is disabled", validation_layer_name);
    }

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{ .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT };
    if (m_Context.enabled_layer_settings.size() > 0) {
        layer_settings_create_info.settingCount = static_cast<uint32_t>(m_Context.enabled_layer_settings.size());
        layer_settings_create_info.pSettings    = m_Context.enabled_layer_settings.data();
        layer_settings_create_info.pNext        = instance_create_info.pNext;
        instance_create_info.pNext              = &layer_settings_create_info;
    }

    VkInstance instance{};
    VK_CHECK_RESULT(vkCreateInstance(&instance_create_info, nullptr, &instance))
    m_Instance.attach(instance);

    volkLoadInstance(m_Instance);

    // === SETUP CONTEXT ===

    m_Context.instance = m_Instance; // instance
}

void fe::RendererVulkan::VKChoosePhysicalDevice() {
    uint32_t gpu_count = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_Instance, &gpu_count, nullptr));

    std::vector<VkPhysicalDevice> physical_devices(gpu_count);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_Instance, &gpu_count, physical_devices.data()));

    // TODO : Choose the best physical device
    uint32_t selected_device = 0; // CHOOSING THE FIRST ONE

    m_PhysicalDevice = physical_devices[selected_device];

    // === SETUP CONTEXT ===

    m_Context.physical_device = m_PhysicalDevice;                                                        // physical device
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Context.physical_device_properties);              // physical device properties
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Context.physical_device_features);                  // physical device features
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_Context.physical_device_memory_properties); // physical device memory properties
}

void fe::RendererVulkan::VKSetupDepthStencilFormat() {

    // === SETUP CONTEXT ===

    VkBool32 found{ false };

    if (m_Context.requires_stencil) {
        std::vector<VkFormat> format_list = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
        };

        for (auto& format : format_list) {
            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &format_properties);

            if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {

                m_Context.depth_format = format; // depth/stencil format
                found                  = true;
            }
        }

        if (!found)
            fe::logging::error("Failed to find suitable depth/stencil format");
    }
    else {
        std::vector<VkFormat> format_list = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };

        for (auto& format : format_list) {
            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &format_properties);

            if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {

                m_Context.depth_format = format; // depth format
                found                  = true;
            }
        }

        if (!found)
            fe::logging::error("Failed to find suitable depth format");
    }
}

void fe::RendererVulkan::VKSetupQueueFamilyProperties() {

    // === SETUP CONTEXT ===

    uint32_t queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, nullptr);
    assert(queue_family_count > 0);
    m_Context.queue_family_properties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_count, m_Context.queue_family_properties.data()); // queue family properties
}

void fe::RendererVulkan::VKSetupSupportedExtensions() {

    // === SETUP CONTEXT ===

    uint32_t extension_count{};
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extension_count, nullptr);

    if (extension_count <= 0) return;

    std::vector<VkExtensionProperties> extension_properties(extension_count);

    VkResult result = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extension_count, &extension_properties.front());
    if (result == VK_SUCCESS) {
        for (auto& e : extension_properties) {
            m_Context.supported_device_extensions.push_back(e.extensionName); // supported extensions
        }
    }
}

void fe::RendererVulkan::VKCreateDevice(bool use_swapchain, VkQueueFlags requested_queue_types) {
    if (use_swapchain) {
        // === SETUP CONTEXT ===
        m_Context.enabled_physical_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        // ===
    }

    // === SETUP CONTEXT ===
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = this->getQueueFamilyInfos(use_swapchain, requested_queue_types);
    // ===

    VkDeviceCreateInfo device_create_info{
        .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos    = queue_create_infos.data(),
        .pEnabledFeatures     = &m_Context.enabled_physical_device_features
    };

    VkPhysicalDeviceFeatures2 physical_device_features2{};
    if (m_Context.physical_device_create_next_chain) {
        physical_device_features2.sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physical_device_features2.features  = m_Context.enabled_physical_device_features;
        physical_device_features2.pNext     = m_Context.physical_device_create_next_chain;
        device_create_info.pEnabledFeatures = nullptr;
        device_create_info.pNext            = &physical_device_features2;
    }

    // must be in this scope
    std::vector<const char*> enabled_physical_device_extensions_cstr{};
    for (const auto& e : m_Context.enabled_physical_device_extensions)
        enabled_physical_device_extensions_cstr.push_back(e.c_str());

    if (!enabled_physical_device_extensions_cstr.empty()) {

        for (auto& e : m_Context.enabled_physical_device_extensions) {

            auto is_extension_supported = [&](const std::string& extension) -> bool {
                return (std::find(m_Context.supported_device_extensions.begin(), m_Context.supported_device_extensions.end(), extension) != m_Context.supported_device_extensions.end());
            };

            if (!is_extension_supported(e)) {
                fe::logging::error("Enabled device extension %s is not present at device level", e);
            }
        }

        device_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_physical_device_extensions_cstr.size());
        device_create_info.ppEnabledExtensionNames = enabled_physical_device_extensions_cstr.data();
    }

    VkDevice device{};
    VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &device));
    m_Device.attach(device);

    volkLoadDevice(device);

    // === SETUP CONTEXT ===
    m_Context.device = m_Device;
}

void fe::RendererVulkan::VKCreateCommandPool(VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo command_pool_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = create_flags,
        .queueFamilyIndex = m_Context.queue_family_indices.graphics // graphics queue family index from m_Context
    };
    VkCommandPool command_pool{};
    VK_CHECK_RESULT(vkCreateCommandPool(m_Device, &command_pool_info, nullptr, &command_pool));
    m_CommandPool.attach(m_Device, command_pool);

    // === SETUP CONTEXT ===
    m_Context.command_pool = m_CommandPool;
}

void fe::RendererVulkan::VKSetupQueues() {
    constexpr static uint32_t queue_index = 0;
    vkGetDeviceQueue(m_Device, m_Context.queue_family_indices.graphics, queue_index, &m_Context.queue_graphics);
    vkGetDeviceQueue(m_Device, m_Context.queue_family_indices.compute, queue_index, &m_Context.queue_compute);
    vkGetDeviceQueue(m_Device, m_Context.queue_family_indices.transfer, queue_index, &m_Context.queue_transfer);
}

void fe::RendererVulkan::VKSetupDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info{};
    descriptor_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_create_info.bindingCount = 1;
    descriptor_layout_create_info.pBindings    = &layout_binding;

    VkDescriptorSetLayout descriptor_set_layout_raw{};
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_Device, &descriptor_layout_create_info, nullptr, &descriptor_set_layout_raw));
    m_DescriptorSetLayout.attach(m_Device, descriptor_set_layout_raw);
}

void fe::RendererVulkan::VKSetupDescriptorPool() {
    VkDescriptorPoolSize descriptor_pool_size[1];
    descriptor_pool_size[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_pool_size[0].descriptorCount = VulkanContext::max_concurrent_frames;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes    = descriptor_pool_size;
    descriptor_pool_create_info.maxSets       = VulkanContext::max_concurrent_frames;

    VkDescriptorPool descriptor_pool_raw{};
    VK_CHECK_RESULT(vkCreateDescriptorPool(m_Device, &descriptor_pool_create_info, nullptr, &descriptor_pool_raw));
    m_DescriptorPool.attach(m_Device, descriptor_pool_raw);
}

void fe::RendererVulkan::VKSetupDescriptorSets() {
    for (size_t i = 0; i < VulkanContext::max_concurrent_frames; i++) {
        VkDescriptorSetLayout descriptor_set_layout_raw = m_DescriptorSetLayout;

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool     = m_DescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        descriptor_set_allocate_info.pSetLayouts        = &descriptor_set_layout_raw;

        VK_CHECK_RESULT(vkAllocateDescriptorSets(m_Device, &descriptor_set_allocate_info, &m_UniformBuffers[i].descriptor_set));

        VkDescriptorBufferInfo descriptor_buffer_info{};
        descriptor_buffer_info.buffer = m_UniformBuffers[i].buffer;
        descriptor_buffer_info.range  = sizeof(ShaderData);

        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet          = m_UniformBuffers[i].descriptor_set;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_set.pBufferInfo     = &descriptor_buffer_info;
        write_descriptor_set.dstBinding      = 0;

        vkUpdateDescriptorSets(m_Device, 1, &write_descriptor_set, 0, nullptr);
    }
}

void fe::RendererVulkan::VKSetupPipelineLayout() {
    VkDescriptorSetLayout descriptor_set_layout_raw = m_DescriptorSetLayout;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts    = &descriptor_set_layout_raw;

    VkPipelineLayout pipeline_layout_raw{};
    VK_CHECK_RESULT(vkCreatePipelineLayout(m_Device, &pipeline_layout_create_info, nullptr, &pipeline_layout_raw));
    m_PipelineLayout.attach(m_Device, pipeline_layout_raw);
}

void fe::RendererVulkan::VKRender() {
    std::array<VkFence, 1> fences{ m_WaitFences[m_CurrentFrame] };

    vkWaitForFences(m_Device, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
    VK_CHECK_RESULT(vkResetFences(m_Device, fences.size(), fences.data()));

    uint32_t image_index{};
    VkResult result = vkAcquireNextImageKHR(m_Device, m_Context.swapchain, UINT64_MAX, m_PresentCompleteSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        this->resizeWindow();
        return;
    }
    else if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR)) {
        fe::logging::error("Failed to acquire the next swapchain image");
        return;
    }

    ShaderData shader_data{};
    shader_data.projection_matrix = m_Camera.getPerspectiveMatrix();
    shader_data.view_matrix       = m_Camera.getViewMatrix();
    shader_data.model_matrix      = glm::mat4(1.0f);

    memcpy(m_UniformBuffers[m_CurrentFrame].mapped, &shader_data, sizeof(ShaderData));

    vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkClearValue clear_values[2]{};
    clear_values[0].color        = { { 0.0f, 0.0f, 0.2f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass               = m_RenderPass;
    render_pass_begin_info.renderArea.offset.x      = 0;
    render_pass_begin_info.renderArea.offset.y      = 0;
    render_pass_begin_info.renderArea.extent.width  = m_Context.swapchain_extent.width;
    render_pass_begin_info.renderArea.extent.height = m_Context.swapchain_extent.height;
    render_pass_begin_info.clearValueCount          = 2;
    render_pass_begin_info.pClearValues             = clear_values;
    render_pass_begin_info.framebuffer              = m_Framebuffers[image_index];

    const VkCommandBuffer command_buffer = m_CommandBuffers[m_CurrentFrame];
    VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width    = (float) m_Context.swapchain_extent.width;
    viewport.height   = (float) m_Context.swapchain_extent.height;
    viewport.minDepth = (float) 0.0f;
    viewport.maxDepth = (float) 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width  = m_Context.swapchain_extent.width;
    scissor.extent.height = m_Context.swapchain_extent.height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_UniformBuffers[m_CurrentFrame].descriptor_set, 0, nullptr);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    VkDeviceSize offsets[1]{ 0 };

    VkBuffer vertex_buffer_raw = m_VertexBuffer.buffer;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer_raw, offsets);

    VkBuffer index_buffer_raw = m_IndexBuffer.buffer;
    vkCmdBindIndexBuffer(command_buffer, index_buffer_raw, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(command_buffer, m_IndexBuffer.count, 1, 0, 0, 0);
    vkCmdEndRenderPass(command_buffer);

    VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));

    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    std::array<VkSemaphore, 1> wait_semaphores{ m_PresentCompleteSemaphores[m_CurrentFrame] };
    std::array<VkSemaphore, 1> signal_semaphores{ m_RenderCompleteSemaphores[image_index] };

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask  = &wait_stage_mask;
    submit_info.pCommandBuffers    = &command_buffer;
    submit_info.commandBufferCount = 1;

    submit_info.pWaitSemaphores    = wait_semaphores.data();
    submit_info.waitSemaphoreCount = wait_semaphores.size();

    submit_info.pSignalSemaphores    = signal_semaphores.data();
    submit_info.signalSemaphoreCount = signal_semaphores.size();

    VK_CHECK_RESULT(vkQueueSubmit(m_Context.queue_graphics, 1, &submit_info, m_WaitFences[m_CurrentFrame]));

    VkPresentInfoKHR present_info{};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores.data();
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &m_Context.swapchain;
    present_info.pImageIndices      = &image_index;

    result = vkQueuePresentKHR(m_Context.queue_graphics, &present_info);

    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        this->resizeWindow();
    }
    else if (result != VK_SUCCESS) {
        fe::logging::error("Failed to present the image to the swapchain");
        return;
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % m_Context.max_concurrent_frames;
}

std::vector<VkDeviceQueueCreateInfo> fe::RendererVulkan::getQueueFamilyInfos(bool use_swapchain, VkQueueFlags requested_queue_types) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};

    constexpr static float default_queue_priority = 1.0f;

    if (requested_queue_types & VK_QUEUE_GRAPHICS_BIT) {
        // === SETUP CONTEXT ===
        m_Context.queue_family_indices.graphics = getQueueFamilyIndex(m_Context, VK_QUEUE_GRAPHICS_BIT);
        // ===
        VkDeviceQueueCreateInfo queue_info{
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_Context.queue_family_indices.graphics,
            .queueCount       = 1,
            .pQueuePriorities = &default_queue_priority
        };
        queue_create_infos.push_back(queue_info);
    }
    else {
        // === SETUP CONTEXT ===
        m_Context.queue_family_indices.graphics = 0;
        // ===
    }

    if (requested_queue_types & VK_QUEUE_COMPUTE_BIT) {
        // === SETUP CONTEXT ===
        m_Context.queue_family_indices.compute = getQueueFamilyIndex(m_Context, VK_QUEUE_COMPUTE_BIT);
        // ===

        auto result = m_Context.queue_family_indices.compute != m_Context.queue_family_indices.graphics;

        if (result) {
            VkDeviceQueueCreateInfo queue_info{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_Context.queue_family_indices.compute,
                .queueCount       = 1,
                .pQueuePriorities = &default_queue_priority,
            };
            queue_create_infos.push_back(queue_info);
        }
    }
    else {
        // === SETUP CONTEXT ===
        m_Context.queue_family_indices.compute = m_Context.queue_family_indices.graphics;
        // ===
    }

    if (requested_queue_types & VK_QUEUE_TRANSFER_BIT) {
        // === SETUP CONTEXT ===
        m_Context.queue_family_indices.transfer = getQueueFamilyIndex(m_Context, VK_QUEUE_TRANSFER_BIT);
        // ===

        auto result = (m_Context.queue_family_indices.transfer != m_Context.queue_family_indices.graphics) &&
                      (m_Context.queue_family_indices.transfer != m_Context.queue_family_indices.compute);

        if (result) {
            VkDeviceQueueCreateInfo queue_info{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_Context.queue_family_indices.transfer,
                .queueCount       = 1,
                .pQueuePriorities = &default_queue_priority
            };
            queue_create_infos.push_back(queue_info);
        }
    }
    else {
        // === SETUP CONTEXT ===
        m_Context.queue_family_indices.transfer = m_Context.queue_family_indices.graphics;
        // ===
    }

    return queue_create_infos;
}

fe::vk::ShaderModule fe::RendererVulkan::createShaderModule(const std::filesystem::path& path) {
    size_t shader_size{};
    char*  shader_code{};

    std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open()) {
        shader_size = is.tellg();
        is.seekg(0, std::ios::beg);
        shader_code = new char[shader_size];
        is.read(shader_code, shader_size);
        is.close();
        assert(shader_size > 0);
    }
    if (shader_code) {
        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = shader_size;
        shader_module_create_info.pCode    = (uint32_t*) shader_code;

        fe::vk::ShaderModule shader_module{};

        VkShaderModule shader_module_raw{};
        VK_CHECK_RESULT(vkCreateShaderModule(m_Device, &shader_module_create_info, nullptr, &shader_module_raw));
        shader_module.attach(m_Device, shader_module_raw);

        delete[] shader_code;

        return std::move(shader_module);
    }

    fe::logging::error("Failed to open shader file. Path : %s", path.string().c_str());

    return fe::vk::ShaderModule{ m_Device, VK_NULL_HANDLE };
}

VKAPI_ATTR VkBool32 VKAPI_CALL fe::RendererVulkan::debugUtilsMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                                                                             VkDebugUtilsMessageTypeFlagsEXT             message_type,
                                                                             const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                             void*                                       user_data) {
    std::string message{};
    message = "[VULKAN VALIDATION]";

    if (callback_data->pMessageIdName) {
        message += "[" + std::to_string(callback_data->messageIdNumber) + "][" + callback_data->pMessageIdName + "] : " + callback_data->pMessage;
    }
    else {
        message += "[" + std::to_string(callback_data->messageIdNumber) + "] : " + callback_data->pMessage;
    }

    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            //fe::logging::info(message.c_str()); --- Turned off
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            //fe::logging::info(message.c_str()); --- Turned off
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            fe::logging::warning(message.c_str());
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            fe::logging::error(message.c_str());
            break;
        default:
            break;
    }

    return VK_FALSE;
}
