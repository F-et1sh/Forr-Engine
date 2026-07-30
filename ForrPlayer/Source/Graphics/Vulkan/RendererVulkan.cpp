/*===============================================

    Forr Engine

    File : RendererVulkn.cpp
    Role : vulkan Renderer implementation

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
                                   size_t              primary_window_index,
                                   ResourceManager&    resource_manager)
    : m_Description(desc),
      m_PlatformSystem(platform_system),
      m_PrimaryWindow(m_PlatformSystem.getWindow(primary_window_index)),
      m_ResourceManager(resource_manager) {

    this->configureCamera();

    this->InitializeBase();
    this->InitializeDevice();
    this->InitializeAllocator();
    this->InitializeSwapchain();
    this->InitializeCommandBuffers();
    this->InitializeSynchronizationPrimitives();
    this->InitializeDepthStencil();
    this->InitializeRenderPass();
    this->InitializeFramebuffers();
    this->InitializeStorageBuffers();
    this->InitializeDescriptors();
}

fe::RendererVulkan::~RendererVulkan() {
    vkDeviceWaitIdle(m_Device);
}

void fe::RendererVulkan::SetClearColor(float red, float green, float blue, float alpha) {
    // === SETUP CONTEXT ===
    m_Context.clear_color = { red, green, blue, alpha }; // clear_color
}

fe::RenderGraphBindings fe::RendererVulkan::CreateGPUResources(const RenderGraphCompileResult& compile_result) {
}

void fe::RendererVulkan::BeginFrame() {
    std::array<VkFence, 1> fences{ m_FrameData[m_CurrentFrame].wait_fence };

    vkWaitForFences(m_Device, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
    VK_CHECK_RESULT(vkResetFences(m_Device, fences.size(), fences.data()));

    m_ImageIndex = 0;

    VkResult result = vkAcquireNextImageKHR(m_Device, m_Context.swapchain, UINT64_MAX, m_FrameData[m_CurrentFrame].present_complete_semaphore, VK_NULL_HANDLE, &m_ImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        this->resizeWindow();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fe::logging::error("Failed to acquire the next swapchain image");
        return;
    }

    vkResetCommandBuffer(m_FrameData[m_CurrentFrame].command_buffer, 0);

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkClearValue clear_values[2]{};
    clear_values[0].color        = { m_Context.clear_color };
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
    render_pass_begin_info.framebuffer              = m_Framebuffers[m_ImageIndex];

    const VkCommandBuffer command_buffer = m_FrameData[m_CurrentFrame].command_buffer;
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
}

void fe::RendererVulkan::EndFrame(const render_graph::CommandList& render_command_list) {
    //this->handleRenderQueue(render_packet);

    const VkCommandBuffer command_buffer = m_FrameData[m_CurrentFrame].command_buffer;

    vkCmdEndRenderPass(command_buffer);

    VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));

    VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    std::array<VkSemaphore, 1> wait_semaphores{ m_FrameData[m_CurrentFrame].present_complete_semaphore };
    std::array<VkSemaphore, 1> signal_semaphores{ m_RenderCompleteSemaphores[m_ImageIndex] };

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask  = &wait_stage_mask;
    submit_info.pCommandBuffers    = &command_buffer;
    submit_info.commandBufferCount = 1;

    submit_info.pWaitSemaphores    = wait_semaphores.data();
    submit_info.waitSemaphoreCount = wait_semaphores.size();

    submit_info.pSignalSemaphores    = signal_semaphores.data();
    submit_info.signalSemaphoreCount = signal_semaphores.size();

    VK_CHECK_RESULT(vkQueueSubmit(m_Context.queue_graphics, 1, &submit_info, m_FrameData[m_CurrentFrame].wait_fence));

    VkPresentInfoKHR present_info{};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores.data();
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = &m_Context.swapchain;
    present_info.pImageIndices      = &m_ImageIndex;

    VkResult result = vkQueuePresentKHR(m_Context.queue_graphics, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        this->resizeWindow();
    }
    else if (result != VK_SUCCESS) {
        fe::logging::error("Failed to present the image to the swapchain");
        return;
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % VulkanContext::max_concurrent_frames;

    // reset
    m_CurrentMaterial = {};
    m_CurrentMesh     = {};
}

void fe::RendererVulkan::InitializeGPUResources() {
    m_ResourceManager.RunForEach<resource::Texture>([&](resource::Texture& texture) {
        m_VulkanResourceManager.CreateResource(texture);

        fe::logging::info("VULKAN. Loaded texture's size : %i %i", texture.width, texture.height);
    });

    m_ResourceManager.RunForEach<resource::Material>([&](resource::Material& material) {
        m_VulkanResourceManager.CreateResource(material);
    });

    m_ResourceManager.RunForEach<resource::Model>([&](resource::Model& model) {
        m_VulkanResourceManager.CreateResource(model);

        fe::logging::info("VULKAN. Loaded model's mesh count %i", model.meshes.size());
    });
}

void fe::RendererVulkan::configureCamera() {
    m_Camera.setType(Camera::Type::LOOKAT);
    m_Camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    m_Camera.setRotation(glm::vec3(0.0f));
    m_Camera.setFlipY(true);

    float fov    = 60.0f;
    float aspect = (float) m_PrimaryWindow.getWidth() / (float) m_PrimaryWindow.getHeight();
    float znear  = 1.0f;
    float zfar   = 1000.0f;
    m_Camera.setPerspective(fov, aspect, znear, zfar);
}

void fe::RendererVulkan::resizeWindow() {
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

void fe::RendererVulkan::handleRenderQueue(const RenderPacket& render_packet) {
    { // temp
        auto glfw_window = (GLFWwindow*) m_PrimaryWindow.getNativeHandle();

        float speed = 0.025f;

        if (glfwGetKey(glfw_window, GLFW_KEY_A))
            m_Camera.translate(glm::vec3(speed, 0.0f, 0.0f));
        else if (glfwGetKey(glfw_window, GLFW_KEY_D))
            m_Camera.translate(glm::vec3(-speed, 0.0f, 0.0f));

        if (glfwGetKey(glfw_window, GLFW_KEY_W))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, speed));
        else if (glfwGetKey(glfw_window, GLFW_KEY_S))
            m_Camera.translate(glm::vec3(0.0f, 0.0f, -speed));

        auto* object_ptr = static_cast<uint8_t*>(m_FrameData[m_CurrentFrame].storage_buffer.bindings[0].mapped);

        struct GPUCamera {
            glm::mat4 p;
            glm::mat4 v;
        } cam{ m_Camera.getPerspectiveMatrix(), m_Camera.getViewMatrix() };
        memcpy(object_ptr, &cam, sizeof(cam));
        object_ptr += sizeof(cam);

        if (!render_packet.object_transforms.empty()) {
            size_t bytes_to_copy = render_packet.object_transforms.size() * sizeof(glm::mat4);
            memcpy(object_ptr, render_packet.object_transforms.data(), bytes_to_copy);
        }

        auto*    lights_ptr   = static_cast<uint8_t*>(m_FrameData[m_CurrentFrame].storage_buffer.bindings[1].mapped);
        uint32_t lights_count = render_packet.lights.size();
        memcpy(lights_ptr, &lights_count, sizeof(lights_count));
        lights_ptr += 16;
        if (!render_packet.lights.empty()) {
            size_t bytes_to_copy = render_packet.lights.size() * sizeof(GPULight);
            memcpy(lights_ptr, render_packet.lights.data(), bytes_to_copy);
        }
    }

    const VkCommandBuffer command_buffer = m_FrameData[m_CurrentFrame].command_buffer;

    // draw
    for (const auto& draw_command : render_packet.draw_commands) {
        const auto& material        = m_ResourceManager.GetResource(draw_command.material_ptr);
        const auto& vulkan_material = m_VulkanResourceManager.GetResource(material->gpu_handle);

        // bind pipeline ( material )
        if (material->gpu_handle != m_CurrentMaterial) {
            m_CurrentMaterial = material->gpu_handle;

            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_material.pipeline);
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_material.pipeline_layout, 0, 1, &m_FrameData[m_CurrentFrame].storage_buffer.descriptor_set, 0, nullptr);
        }

        // bind vertex and index buffers
        if (draw_command.mesh_handle != m_CurrentMesh) {
            m_CurrentMesh = draw_command.mesh_handle;

            const auto& vulkan_mesh = m_VulkanResourceManager.GetResource(m_CurrentMesh);

            VkDeviceSize offsets[1]{ 0 };

            VkBuffer vertex_buffer_raw = vulkan_mesh.vertex_buffer.get<VkBuffer>();
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer_raw, offsets);

            VkBuffer index_buffer_raw = vulkan_mesh.index_buffer.get<VkBuffer>();
            vkCmdBindIndexBuffer(command_buffer, index_buffer_raw, 0, VK_INDEX_TYPE_UINT32);
        }

        uint32_t constants = draw_command.instance_index;
        vkCmdPushConstants(command_buffer, vulkan_material.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &constants);

        vkCmdDrawIndexed(command_buffer, draw_command.index_count, 1, draw_command.index_offset, 0, 0);
    }
}

void fe::RendererVulkan::InitializeBase() {
    VK_CHECK_RESULT(volkInitialize());

    m_Context.enabled_instance_extensions.emplace_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

    this->VKCreateInstance();
    this->VKChoosePhysicalDevice();
    this->VKSetupDepthStencilFormat();

    // TODO : After VKChoosePhysicalDevice() you getting
    // m_Context::physical_device_features - they are not enabled.
    // So, it's needed to provide ability to enable some of them if you need
}

void fe::RendererVulkan::InitializeDevice() {
    this->VKSetupFeatures();

    this->VKSetupQueueFamilyProperties();
    this->VKSetupSupportedExtensions();

    // TODO : Add enabled extensions adding

    m_Context.enabled_physical_device_extensions.emplace_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    m_Context.enabled_physical_device_extensions.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

    this->VKCreateDevice();
    this->VKCreateCommandPool();
    this->VKSetupQueues();
}

void fe::RendererVulkan::InitializeAllocator() {
    VmaAllocatorCreateInfo allocator_create_info{};
    allocator_create_info.physicalDevice   = m_PhysicalDevice;
    allocator_create_info.device           = m_Device;
    allocator_create_info.instance         = m_Instance;
    allocator_create_info.vulkanApiVersion = m_Context.api_version;
    //allocator_create_info.flags            = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
    allocator_create_info.flags = 0;

#ifdef _WIN32
    //allocator_create_info.flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
#endif

    VmaVulkanFunctions vulkan_functions{};
    VK_CHECK_RESULT(vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vulkan_functions));
    allocator_create_info.pVulkanFunctions = &vulkan_functions;

    VmaAllocator allocator{};
    VK_CHECK_RESULT(vmaCreateAllocator(&allocator_create_info, &allocator));

    m_Allocator.attach(allocator);

    // === SETUP CONTEXT ===
    m_Context.allocator = m_Allocator.get(); // allocator
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
    command_buffer_allocate_info.commandBufferCount = 1; // one per frame

    for (auto& frame : m_FrameData) {
        VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, &frame.command_buffer));
    }
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

        m_FrameData[i].wait_fence.attach(m_Device, fence);
    }

    ///

    for (size_t i = 0; i < present_complete_semaphores_raw.size(); i++) {
        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        auto& semaphore = present_complete_semaphores_raw[i];
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, nullptr, &semaphore));

        m_FrameData[i].present_complete_semaphore.attach(m_Device, semaphore);
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

void fe::RendererVulkan::InitializeStorageBuffers() {
    constexpr static size_t binding0_size = sizeof(glm::mat4) * 2 + sizeof(glm::mat4) * 100'000;
    constexpr static size_t binding1_size = sizeof(GPULight) * 100'000;

    // TODO : move into a helper-function
    auto initialize_binding = [&](std::size_t frame_data_i, std::size_t binding_i, std::size_t buffer_size) {
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size  = buffer_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBuffer buffer_raw{};
        VK_CHECK_RESULT(vkCreateBuffer(m_Device, &buffer_create_info, nullptr, &buffer_raw));
        m_FrameData[frame_data_i].storage_buffer.bindings[binding_i].buffer.attach(m_Device, buffer_raw);

        VkMemoryRequirements memory_requirements{};
        vkGetBufferMemoryRequirements(m_Device, buffer_raw, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize  = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex = getMemoryTypeIndex(m_Context, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkDeviceMemory memory_raw{};
        VK_CHECK_RESULT(vkAllocateMemory(m_Device, &memory_allocate_info, nullptr, &memory_raw));
        m_FrameData[frame_data_i].storage_buffer.bindings[binding_i].memory.attach(m_Device, memory_raw);

        constexpr static VkDeviceSize     offset = 0;
        constexpr static VkMemoryMapFlags flags  = 0;

        VK_CHECK_RESULT(vkBindBufferMemory(m_Device, buffer_raw, memory_raw, offset));
        VK_CHECK_RESULT(vkMapMemory(m_Device, memory_raw, offset, VK_WHOLE_SIZE, flags, (void**) &m_FrameData[frame_data_i].storage_buffer.bindings[binding_i].mapped));
    };

    for (size_t i = 0; i < VulkanContext::max_concurrent_frames; i++) {
        m_FrameData[i].storage_buffer.bindings.resize(2);

        initialize_binding(i, 0, binding0_size);
        initialize_binding(i, 1, binding1_size);
    }
}

void fe::RendererVulkan::InitializeDescriptors() {
    this->VKSetupDescriptorSetLayout();
    this->VKSetupDescriptorPool();
    this->VKSetupDescriptorSets();
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

        auto it = std::ranges::find(m_Context.supported_instance_extensions, e);

        if (it == m_Context.supported_instance_extensions.end())
            extensions_to_remove.push_back(i);
    }

    for (auto i : extensions_to_remove) {
        auto it = m_Context.enabled_instance_extensions.begin() + i;
        m_Context.enabled_instance_extensions.erase(it);
    }

    if constexpr (true /* m_ShaderDir == "slang" */) {
        if constexpr (m_Context.api_version < VK_API_VERSION_1_1) { // this is hardcoded for now. It will be false always
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

void fe::RendererVulkan::VKSetupFeatures() {
    // === SETUP CONTEXT ===

    if (m_Context.api_version <= VK_VERSION_1_1)
        m_Context.enabled_physical_device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME); // enabled_physical_device_extensions

    auto& descriptor_indexing_features = m_Context.enabled_physical_device_descriptor_indexing_features;

    descriptor_indexing_features       = {}; // reset before setting
    descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

    // for bindless
    descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing     = VK_TRUE;
    descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind  = VK_TRUE;
    descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    descriptor_indexing_features.descriptorBindingPartiallyBound               = VK_TRUE;
    descriptor_indexing_features.descriptorBindingVariableDescriptorCount      = VK_TRUE;
    descriptor_indexing_features.runtimeDescriptorArray                        = VK_TRUE;
    descriptor_indexing_features.shaderStorageImageArrayNonUniformIndexing     = VK_TRUE;
    descriptor_indexing_features.descriptorBindingStorageImageUpdateAfterBind  = VK_TRUE;

    m_Context.physical_device_create_next_chain = &descriptor_indexing_features; // physical_device_create_next_chain
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
    std::vector<VkDescriptorSetLayoutBinding> bindings(2);
    std::vector<VkDescriptorBindingFlags>     binding_flags(2);

    bindings[0].binding         = 0;
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    binding_flags[0] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    bindings[1].binding         = 1;
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    binding_flags[1] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_create_info{};
    binding_flags_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    binding_flags_create_info.bindingCount  = binding_flags.size();
    binding_flags_create_info.pBindingFlags = binding_flags.data();

    VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info{};
    descriptor_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_create_info.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    descriptor_layout_create_info.bindingCount = bindings.size();
    descriptor_layout_create_info.pBindings    = bindings.data();
    descriptor_layout_create_info.pNext        = &binding_flags_create_info;

    VkDescriptorSetLayout descriptor_set_layout_raw{};
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_Device, &descriptor_layout_create_info, nullptr, &descriptor_set_layout_raw));
    m_GlobalDescriptorSetLayout.attach(m_Device, descriptor_set_layout_raw);

    // === SETUP CONTEXT ===
    m_Context.global_descriptor_set_layout = m_GlobalDescriptorSetLayout.get();
}

void fe::RendererVulkan::VKSetupDescriptorPool() {
    VkDescriptorPoolSize pool_size{};
    pool_size.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_size.descriptorCount = m_Context.max_concurrent_frames;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.maxSets       = m_Context.max_concurrent_frames;
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes    = &pool_size;
    descriptor_pool_create_info.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    VkDescriptorPool descriptor_pool_raw{};
    VK_CHECK_RESULT(vkCreateDescriptorPool(m_Device, &descriptor_pool_create_info, nullptr, &descriptor_pool_raw));
    m_DescriptorPool.attach(m_Device, descriptor_pool_raw);
}

void fe::RendererVulkan::VKSetupDescriptorSets() {
    for (size_t i = 0; i < VulkanContext::max_concurrent_frames; i++) {
        VkDescriptorSetLayout descriptor_set_layout_raw = m_Context.global_descriptor_set_layout;

        //uint32_t                                           max_binding_count = 100'000;
        //VkDescriptorSetVariableDescriptorCountAllocateInfo descriptor_set_variable_descriptor_count_allocate_info{};
        //descriptor_set_variable_descriptor_count_allocate_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        //descriptor_set_variable_descriptor_count_allocate_info.descriptorSetCount = 1;
        //descriptor_set_variable_descriptor_count_allocate_info.pDescriptorCounts  = &max_binding_count;

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool     = m_DescriptorPool;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        descriptor_set_allocate_info.pSetLayouts        = &descriptor_set_layout_raw;
        descriptor_set_allocate_info.pNext              = nullptr;

        VK_CHECK_RESULT(vkAllocateDescriptorSets(m_Device, &descriptor_set_allocate_info, &m_FrameData[i].storage_buffer.descriptor_set));

        std::vector<VkWriteDescriptorSet>   write_descriptor_sets(2);
        std::vector<VkDescriptorBufferInfo> descriptor_buffer_infos(2);

        auto setup_write_descriptor_set = [&](std::size_t binding_i) {
            descriptor_buffer_infos[binding_i].buffer = m_FrameData[i].storage_buffer.bindings[binding_i].buffer;
            descriptor_buffer_infos[binding_i].offset = 0;
            descriptor_buffer_infos[binding_i].range  = VK_WHOLE_SIZE;

            write_descriptor_sets[binding_i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_sets[binding_i].dstSet          = m_FrameData[i].storage_buffer.descriptor_set;
            write_descriptor_sets[binding_i].dstBinding      = binding_i;
            write_descriptor_sets[binding_i].descriptorCount = 1;
            write_descriptor_sets[binding_i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write_descriptor_sets[binding_i].pBufferInfo     = &descriptor_buffer_infos[binding_i];
        };

        setup_write_descriptor_set(0);
        setup_write_descriptor_set(1);

        vkUpdateDescriptorSets(m_Device, write_descriptor_sets.size(), write_descriptor_sets.data(), 0, nullptr);
    }
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
