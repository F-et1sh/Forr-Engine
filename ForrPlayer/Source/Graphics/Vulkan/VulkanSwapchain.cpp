/*===============================================

    Forr Engine

    File : VulkanSwapchain.cpp
    Role : Separating some objects from main logic.
        This class can edit main VulkanContext

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "VulkanSwapchain.hpp"

void fe::VulkanSwapchain::CreateSurface() {
    // needed to call in default case
    auto create_glfw_surface = [&]() {
        VkSurfaceKHR surface{};
        VK_CHECK_RESULT(glfwCreateWindowSurface(m_Context.instance, static_cast<GLFWwindow*>(m_PrimaryWindow.getNativeHandle()), nullptr, &surface))
        m_Surface.attach(m_Context.instance, surface);

        // === SETUP CONTEXT ===
        m_Context.surface = m_Surface;
        // ===
    };

    switch (m_RendererDescription.platform_backend) {
        case PlatformBackend::GLFW:

            create_glfw_surface();

            break;
        default:
            fe::logging::error("Failed to create Vulkan surface. Unknown platform backend %i. Using the default one - GLFW", m_RendererDescription.platform_backend);

            create_glfw_surface();
            break;
    }
}

void fe::VulkanSwapchain::SetupSurfaceColorFormat() {
    uint32_t format_count{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.physical_device, m_Surface, &format_count, nullptr));
    assert(format_count > 0);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context.physical_device, m_Surface, &format_count, surface_formats.data()));

    VkSurfaceFormatKHR selected_format = surface_formats[0];

    std::vector<VkFormat> preferred_image_formats = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32
    };

    for (auto& available_format : surface_formats) {
        auto it = std::find(preferred_image_formats.begin(), preferred_image_formats.end(), available_format.format);
        if (it != preferred_image_formats.end()) {
            selected_format = available_format;
            break;
        }
    }

    m_ColorFormat = selected_format.format;
    m_ColorSpace  = selected_format.colorSpace;

    // === SETUP CONTEXT ===
    m_Context.swapchain_color_format = m_ColorFormat;
}

void fe::VulkanSwapchain::SetupQueueNodeIndex() {
    // TODO : Support present queue
    m_QueueNodeIndex = m_Context.queue_family_indices.graphics;
}

void fe::VulkanSwapchain::CreateSwapchain() {
    VkSwapchainKHR old_swapchain = m_Swapchain;

    VkSurfaceCapabilitiesKHR surface_capabilities{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context.physical_device, m_Surface, &surface_capabilities));

    VkExtent2D swapchain_extent{};
    swapchain_extent.width  = m_PrimaryWindow.getWidth();
    swapchain_extent.height = m_PrimaryWindow.getHeight();

    if (surface_capabilities.currentExtent.width == ~0 &&
        surface_capabilities.currentExtent.height == ~0) {

        std::clamp(swapchain_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        std::clamp(swapchain_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

        m_PrimaryWindow.setResolution(swapchain_extent.width, swapchain_extent.height);
    }
    else {
        swapchain_extent = surface_capabilities.currentExtent;

        if (swapchain_extent.width != m_PrimaryWindow.getWidth() ||
            swapchain_extent.height != m_PrimaryWindow.getHeight()) {

            fe::logging::warning("Problem with resolution. Swapchain extent is %ix%i but window extent is %ix%i",
                                 swapchain_extent.width,
                                 swapchain_extent.height,
                                 m_PrimaryWindow.getWidth(),
                                 m_PrimaryWindow.getHeight());
        }
    }

    m_Extent = swapchain_extent; // set swapchain extent
    
    // === SETUP CONTEXT ===
    m_Context.swapchain_extent = m_Extent;
    // ===

    uint32_t present_mode_count{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context.physical_device, m_Surface, &present_mode_count, nullptr));
    assert(present_mode_count > 0);

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context.physical_device, m_Surface, &present_mode_count, present_modes.data()));

    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if (m_PrimaryWindow.getVSync() == 0) {
        for (size_t i = 0; i < present_mode_count; i++) {

            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    uint32_t min_image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 &&
        min_image_count > surface_capabilities.maxImageCount) {

        min_image_count = surface_capabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR transform{};
    if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
        transform = surface_capabilities.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& e : composite_alpha_flags) {
        if (surface_capabilities.supportedCompositeAlpha & e) {
            composite_alpha = e;
            break;
        };
    }

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface          = m_Surface;
    swapchain_create_info.minImageCount    = min_image_count;
    swapchain_create_info.imageFormat      = m_ColorFormat;
    swapchain_create_info.imageColorSpace  = m_ColorSpace;
    swapchain_create_info.imageExtent      = m_Extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE; // TODO : Support separate present queue // VK_SHARING_MODE_CONCURRENT
    swapchain_create_info.queueFamilyIndexCount = 1;                         // TODO : Support separate present queue // 2
    swapchain_create_info.pQueueFamilyIndices   = &m_QueueNodeIndex;         // TODO : Support separate present queue // { graphicsIndex, presentIndex }

    swapchain_create_info.preTransform   = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
    swapchain_create_info.compositeAlpha = composite_alpha;
    swapchain_create_info.presentMode    = swapchain_present_mode;
    swapchain_create_info.clipped        = VK_TRUE;
    swapchain_create_info.oldSwapchain   = old_swapchain;

    if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkSwapchainKHR swapchain{};
    VK_CHECK_RESULT(vkCreateSwapchainKHR(m_Context.device, &swapchain_create_info, nullptr, &swapchain));
    m_Swapchain.attach(m_Context.device, swapchain);

    // === SETUP CONTEXT ===
    m_Context.swapchain = swapchain;
    // ===

    // get images count
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_Context.device, m_Swapchain, &m_ImageCount, nullptr));

    // resize
    m_Images.resize(m_ImageCount);
    m_ImageViews.resize(m_ImageCount);

    // === SETUP CONTEXT ===
    m_Context.swapchain_image_count = m_ImageCount;
    // ===

    // get images
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_Context.device, m_Swapchain, &m_ImageCount, m_Images.data()));

    // not RAII image views
    std::vector<VkImageView> image_views{};
    image_views.resize(m_ImageCount);

    for (size_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo color_attachment_view{};
        color_attachment_view.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        color_attachment_view.image            = m_Images[i],
        color_attachment_view.viewType         = VK_IMAGE_VIEW_TYPE_2D,
        color_attachment_view.format           = m_ColorFormat,
        color_attachment_view.components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
        color_attachment_view.subresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        };

        VK_CHECK_RESULT(vkCreateImageView(m_Context.device, &color_attachment_view, nullptr, &image_views[i]));
        m_ImageViews[i].attach(m_Context.device, image_views[i]);
    }
}
