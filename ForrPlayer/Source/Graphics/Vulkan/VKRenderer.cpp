// temp
#include "pch.hpp"
#include "VKRenderer.hpp"

void fe::VKRenderer::Initialize(GLFWwindow* glfw_window) {
    if (useDynamicRendering && apiVersion < VK_API_VERSION_1_3) {
        fe::logging::error("Using the core variant of dynamic rendering requires Vulkan 1.3");
        exit(-1);
    }

    createSurface(glfw_window);
    createCommandPool();
    createSwapchain();
    createCommandBuffers();
    createSynchronizationPrimitives();
    setupDepthStencil();
    setupRenderPass();
    createPipelineCache();
    setupFrameBuffer();

    //settings.overlay = settings.overlay && (!benchmark.active);
    //if (settings.overlay) {
    //    ui.maxConcurrentFrames = maxConcurrentFrames;
    //    ui.device              = vulkanDevice;
    //    ui.queue               = queue;
    //    ui.shaders             = {
    //        loadShader(getShadersPath() + "base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
    //        loadShader(getShadersPath() + "base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
    //    };
    //    ui.prepareResources();
    //    ui.preparePipeline(pipelineCache, renderPass, swapChain.colorFormat, depthFormat);
    //}
}

void fe::VKRenderer::createSurface(GLFWwindow* glfw_window) {
    swapchain.initSurface(glfw_window);
    //#if defined(_WIN32)
    //	swapChain.initSurface(windowInstance, window);
    //#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    //	swapChain.initSurface(androidApp->window);
    //#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
    //	swapChain.initSurface(view);
    //#elif defined(VK_USE_PLATFORM_METAL_EXT)
    //	swapChain.initSurface(metalLayer);
    //#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    //	swapChain.initSurface(dfb, surface);
    //#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    //	swapChain.initSurface(display, surface);
    //#elif defined(VK_USE_PLATFORM_XCB_KHR)
    //	swapChain.initSurface(connection, window);
    //#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
    //	swapChain.initSurface(width, height);
    //#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    //	swapChain.initSurface(screen_context, screen_window);
    //#endif
}

void fe::VKRenderer::createCommandPool() {
    VkCommandPoolCreateInfo cmdPoolInfo{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = swapchain.queueNodeIndex,
    };
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
}

void fe::VKRenderer::createSwapchain() {
    struct {
        bool vsync      = 1;
        bool fullscreen = 1;
    } settings; // TODO : Move this to desc

    swapchain.create(width, height, settings.vsync, settings.fullscreen);
}

void fe::VKRenderer::createCommandBuffers() {
    VkCommandBufferAllocateInfo cmdBufAllocateInfo{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = cmdPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(drawCmdBuffers.size()),
    };
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
}

void fe::VKRenderer::createSynchronizationPrimitives() {
    // Wait fences to sync command buffer access
    VkFenceCreateInfo fenceCreateInfo{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };
    for (auto& fence : waitFences) {
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
    }
    // Used to ensure that image presentation is complete before starting to submit again
    for (auto& semaphore : presentCompleteSemaphores) {
        VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
    }
    // Semaphore used to ensure that all commands submitted have been finished before submitting the image to the queue
    renderCompleteSemaphores.resize(swapchain.images.size());
    for (auto& semaphore : renderCompleteSemaphores) {
        VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
    }
}

void fe::VKRenderer::setupDepthStencil() {
    VkImageCreateInfo imageCI{
        .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType   = VK_IMAGE_TYPE_2D,
        .format      = depthFormat,
        .extent      = { width, height, 1 },
        .mipLevels   = 1,
        .arrayLayers = 1,
        .samples     = VK_SAMPLE_COUNT_1_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    };
    VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &depthStencil.image));
    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);

    VkMemoryAllocateInfo memAllloc{
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize  = memReqs.size,
        //.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // TODO : Create Vulkan device
    };
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllloc, nullptr, &depthStencil.memory));
    VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.memory, 0));

    VkImageViewCreateInfo imageViewCI{
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = depthStencil.image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = depthFormat,
        .subresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        }
    };
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCI, nullptr, &depthStencil.view));
}

void fe::VKRenderer::setupRenderPass() {
    if (useDynamicRendering) {
        // When dynamic rendering is enabled, render passes are no longer required
        renderPass = VK_NULL_HANDLE;
        return;
    }
    std::array<VkAttachmentDescription, 2> attachments{
        // Color attachment
        VkAttachmentDescription{
            .format         = swapchain.colorFormat,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
        // Depth attachment
        VkAttachmentDescription{
            .format         = depthFormat,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
    };

    VkAttachmentReference colorReference{ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference{ .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription{
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount    = 1,
        .pColorAttachments       = &colorReference,
        .pDepthStencilAttachment = &depthReference,
    };

    // Subpass dependencies for layout transitions
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

    VkRenderPassCreateInfo renderPassInfo{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments    = attachments.data(),
        .subpassCount    = 1,
        .pSubpasses      = &subpassDescription,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies   = dependencies.data(),
    };
    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void fe::VKRenderer::createPipelineCache() {
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
    VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

void fe::VKRenderer::setupFrameBuffer() {
    if (useDynamicRendering) {
        // When dynamic rendering is enabled, render passes are no longer required
        renderPass = VK_NULL_HANDLE;
        return;
    }
    // Create frame buffers for every swap chain image, only one depth/stencil attachment is required, as this is owned by the application
    frameBuffers.resize(swapchain.images.size());
    for (uint32_t i = 0; i < frameBuffers.size(); i++) {
        const VkImageView       attachments[2] = { swapchain.imageViews[i], depthStencil.view };
        VkFramebufferCreateInfo frameBufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = renderPass,
            .attachmentCount = 2,
            .pAttachments    = attachments,
            .width           = width,
            .height          = height,
            .layers          = 1
        };
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
    }
}
