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

VkShaderModule fe::RendererVulkan::create_shader_module(const std::string& code) {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = code.size();
    shader_module_create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module{};
    VK_CHECK_RESULT(vkCreateShaderModule(m_Device, &shader_module_create_info, nullptr, &shader_module));

    return shader_module;
}

std::string fe::RendererVulkan::load_shader(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.good()) {
        fe::logging::error("Failed to load shader file\nPath : %s", path.string().c_str());
        return "";
    }
    return { (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>() };
}

void fe::RendererVulkan::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index, VkRenderPass render_pass, std::vector<VkFramebuffer> swapchain_framebuffers, VkExtent2D swapchain_extent, VkPipeline graphics_pipeline) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &begin_info));

    VkClearValue clear_values = { { 0.025f, 0.025f, 0.025f, 1.0f } };

    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass        = render_pass;
    render_pass_begin_info.framebuffer       = swapchain_framebuffers[image_index];
    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.renderArea.extent = swapchain_extent;
    render_pass_begin_info.clearValueCount   = 1;
    render_pass_begin_info.pClearValues      = &clear_values;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = swapchain_extent.width;
    viewport.height   = swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_extent;

    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer     vertex_buffers[] = { m_VertexBuffer };
    VkDeviceSize offsets[]       = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(command_buffer, m_Indices.size(), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);
    VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));
}

uint32_t fe::RendererVulkan::findMemoryType(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void fe::RendererVulkan::createBuffer(VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(m_Device, &buffer_info, nullptr, &buffer))

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize  = memory_requirements.size;
    allocate_info.memoryTypeIndex = findMemoryType(physical_device, memory_requirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(vkAllocateMemory(m_Device, &allocate_info, nullptr, &buffer_memory))
    VK_CHECK_RESULT(vkBindBufferMemory(m_Device, buffer, buffer_memory, 0));
}

VkCommandBuffer fe::RendererVulkan::beginSingleTimeCommands(VkCommandPool command_pool) {
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool        = command_pool;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_Device, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &beginInfo));

    return command_buffer;
}

void fe::RendererVulkan::endSingleTimeCommands(VkQueue graphics_queue, VkCommandPool command_pool, VkCommandBuffer command_buffer) {
    VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &command_buffer;

    VK_CHECK_RESULT(vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE));
    VK_CHECK_RESULT(vkQueueWaitIdle(graphics_queue));

    vkFreeCommandBuffers(m_Device, command_pool, 1, &command_buffer);
}

void fe::RendererVulkan::copyBuffer(VkQueue graphics_queue, VkCommandPool command_pool, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
    VkCommandBuffer command_buffer = beginSingleTimeCommands(command_pool);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    endSingleTimeCommands(graphics_queue, command_pool, command_buffer);
}

void fe::RendererVulkan::createVertexBuffer(VkPhysicalDevice physical_device, VkQueue graphics_queue, VkCommandPool command_pool) {
    VkDeviceSize buffer_size = sizeof(m_Vertices[0]) * m_Vertices.size();

    VkBuffer       staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(m_Device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, m_Vertices.data(), (size_t) buffer_size);
    vkUnmapMemory(m_Device, staging_buffer_memory);

    createBuffer(physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

    copyBuffer(graphics_queue, command_pool, staging_buffer, m_VertexBuffer, buffer_size);

    vkDestroyBuffer(m_Device, staging_buffer, nullptr);
    vkFreeMemory(m_Device, staging_buffer_memory, nullptr);
}

void fe::RendererVulkan::createIndexBuffer(VkPhysicalDevice physical_device, VkQueue graphics_queue, VkCommandPool command_pool) {
    VkDeviceSize buffer_size = sizeof(m_Indices[0]) * m_Indices.size();

    VkBuffer       staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    createBuffer(physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(m_Device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, m_Indices.data(), (size_t) buffer_size);
    vkUnmapMemory(m_Device, staging_buffer_memory);

    createBuffer(physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

    copyBuffer(graphics_queue, command_pool, staging_buffer, m_IndexBuffer, buffer_size);

    vkDestroyBuffer(m_Device, staging_buffer, nullptr);
    vkFreeMemory(m_Device, staging_buffer_memory, nullptr);
}

fe::RendererVulkan::RendererVulkan(const RendererDesc& desc,
                                   IPlatformSystem&    platform_system,
                                   size_t              primary_window_index)
    : m_PlatformSystem(platform_system),
      m_PrimaryWindow(m_PlatformSystem.getWindow(primary_window_index)) {

    VK_CHECK_RESULT(volkInitialize());

    m_GLFWwindow = (GLFWwindow*) m_PrimaryWindow.getNativeHandle();

    uint32_t                 glfw_extensions_count = 0;
    const char**             glfw_extensions       = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extensions_count);

    VkApplicationInfo app_info{};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Forr Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "Forr Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info{};
    create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledExtensionCount   = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();

    VK_CHECK_RESULT(vkCreateInstance(&create_info, nullptr, (VkInstance*)&m_Instance))
    volkLoadInstance(m_Instance);

    VkSurfaceKHR surface{};
    VK_CHECK_RESULT(glfwCreateWindowSurface(m_Instance, m_GLFWwindow, nullptr, &surface))

    uint32_t device_count = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_Instance, &device_count, nullptr));
    if (device_count == 0) {
        fe::logging::error("Failed to initialize Vulkan Renderer. No Vulkan devices found");
        return;
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_Instance, &device_count, devices.data()));

    VkPhysicalDevice physical_device = devices[0]; // CHOOSING FIRST DEVICE

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    uint32_t graphics_queue_family_index = 0;
    uint32_t present_queue_family_index  = 0;

    for (size_t i = 0; i < queue_families.size(); i++) {
        VkQueueFamilyProperties queue_family = queue_families[i];

        VkBool32 present_support = false;
        VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support))

        if (present_support) present_queue_family_index = i;

        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family_index = i;
            break;
        }
    }

    if (graphics_queue_family_index == UINT32_MAX) {
        fe::logging::error("Failed to initialize Vulkan Renderer. No graphics queue found");
        return;
    }

    VkPhysicalDeviceFeatures device_features{};

    float queue_priority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::unordered_set<uint32_t>         unique_queue_families = {
        graphics_queue_family_index,
        present_queue_family_index
    };

    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueCount       = 1;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.pQueuePriorities = &queue_priority;

        queue_create_infos.emplace_back(queue_create_info);
    }

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    const std::vector<const char*> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    device_create_info.enabledExtensionCount   = device_extensions.size();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.pEnabledFeatures        = &device_features;

    device_create_info.queueCreateInfoCount = queue_create_infos.size();
    device_create_info.pQueueCreateInfos    = queue_create_infos.data();

    VK_CHECK_RESULT(vkCreateDevice(physical_device, &device_create_info, nullptr, (VkDevice*)&m_Device))

    VkQueue graphics_queue{};
    VkQueue present_queue{};
    vkGetDeviceQueue(m_Device, graphics_queue_family_index, 0, &graphics_queue);
    vkGetDeviceQueue(m_Device, present_queue_family_index, 0, &present_queue);

    uint32_t format_count = 0;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr));
    std::vector<VkSurfaceFormatKHR> swapchain_formats(format_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, swapchain_formats.data()));

    VkSurfaceFormatKHR surface_format = swapchain_formats[0];
    for (VkSurfaceFormatKHR avaliable_format : swapchain_formats) {
        if (avaliable_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && avaliable_format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            surface_format = avaliable_format;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities));
    VkExtent2D extent      = capabilities.currentExtent;
    uint32_t   image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
        image_count = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface          = surface;
    swapchain_create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.clipped          = VK_TRUE;
    swapchain_create_info.oldSwapchain     = VK_NULL_HANDLE;
    swapchain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageArrayLayers = 1;

    uint32_t queue_family_indices[] = { graphics_queue_family_index, present_queue_family_index };
    if (graphics_queue_family_index != present_queue_family_index) {
        swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices   = queue_family_indices;
    }
    else
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    uint32_t present_mode_count = 0;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr));
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data()));

    VkPresentModeKHR preset_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    for (VkPresentModeKHR avaliable_mode : present_modes) {
        if (avaliable_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            preset_mode = avaliable_mode;
            break;
        }
    }
    swapchain_create_info.presentMode     = preset_mode;
    swapchain_create_info.imageExtent     = extent;
    swapchain_create_info.minImageCount   = image_count;
    swapchain_create_info.preTransform    = capabilities.currentTransform;
    swapchain_create_info.imageFormat     = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;

    VkSwapchainKHR swapchain{};
    VK_CHECK_RESULT(vkCreateSwapchainKHR(m_Device, &swapchain_create_info, nullptr, &swapchain));

    uint32_t swapchain_image_count = 0;
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_Device, swapchain, &swapchain_image_count, nullptr));
    std::vector<VkImage> swapchain_images(swapchain_image_count);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_Device, swapchain, &swapchain_image_count, swapchain_images.data()));

    VkFormat                 swapchain_image_format = surface_format.format;
    VkExtent2D               swapchain_extent       = extent;
    std::vector<VkImageView> swapchain_image_views(swapchain_images.size());

    for (size_t i = 0; i < swapchain_images.size(); i++) {
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image                           = swapchain_images[i];
        image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format                          = swapchain_image_format;
        image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel   = 0;
        image_view_create_info.subresourceRange.levelCount     = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount     = 1;

        VK_CHECK_RESULT(vkCreateImageView(m_Device, &image_view_create_info, nullptr, &swapchain_image_views[i]));
    }

    VkAttachmentDescription color_attachment_description{};
    color_attachment_description.format         = swapchain_image_format;
    color_attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_description.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference{};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments    = &color_attachment_reference;
    subpass_description.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass    = 0;
    subpass_dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments    = &color_attachment_description;
    render_pass_create_info.subpassCount    = 1;
    render_pass_create_info.pSubpasses      = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies   = &subpass_dependency;

    VkRenderPass render_pass{};
    VK_CHECK_RESULT(vkCreateRenderPass(m_Device, &render_pass_create_info, nullptr, &render_pass));

    std::string vert_source = load_shader(PATH.getShadersPath() / L"default.vert.spv");
    std::string frag_source = load_shader(PATH.getShadersPath() / L"default.frag.spv");

    VkShaderModule vert_shader_module = create_shader_module(vert_source);
    VkShaderModule frag_shader_module = create_shader_module(frag_source);

    VkPipelineShaderStageCreateInfo vert_pipeline_shader_stage_create_info{};
    vert_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_pipeline_shader_stage_create_info.module = vert_shader_module;
    vert_pipeline_shader_stage_create_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_pipeline_shader_stage_create_info{};
    frag_pipeline_shader_stage_create_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_pipeline_shader_stage_create_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_pipeline_shader_stage_create_info.module = frag_shader_module;
    frag_pipeline_shader_stage_create_info.pName  = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_pipeline_shader_stage_create_info,
        frag_pipeline_shader_stage_create_info
    };

    VkVertexInputBindingDescription binding{}; // VERTICES
    binding.binding   = 0;
    binding.stride    = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute{};
    attribute.binding  = 0;
    attribute.location = 0;
    attribute.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute.offset   = offsetof(Vertex, position);

    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
    pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
    pipeline_vertex_input_state_create_info.pVertexBindingDescriptions    = &binding;

    pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = 1;
    pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions    = &attribute;

    VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
    pipeline_input_assembly_state_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipeline_input_assembly_state_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{};
    pipeline_viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_viewport_state_create_info.viewportCount = 1;
    pipeline_viewport_state_create_info.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
    pipeline_rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_rasterization_state_create_info.depthClampEnable        = VK_FALSE;
    pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
    pipeline_rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
    pipeline_rasterization_state_create_info.lineWidth               = 1.0f;
    pipeline_rasterization_state_create_info.cullMode                = VK_CULL_MODE_FRONT_BIT;
    pipeline_rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
    pipeline_multisample_state_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipeline_multisample_state_create_info.sampleShadingEnable  = VK_FALSE;

    VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state{};
    pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_color_blend_attachment_state.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
    pipeline_color_blend_state_create_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipeline_color_blend_state_create_info.logicOpEnable   = VK_FALSE;
    pipeline_color_blend_state_create_info.attachmentCount = 1;
    pipeline_color_blend_state_create_info.pAttachments    = &pipeline_color_blend_attachment_state;

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
    pipeline_dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
    pipeline_dynamic_state_create_info.pDynamicStates    = dynamic_states.data();

    VkPipelineLayout           pipeline_layout{};
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount         = 0;
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    VK_CHECK_RESULT(vkCreatePipelineLayout(m_Device, &pipeline_layout_create_info, nullptr, &pipeline_layout));

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.stageCount          = 2;
    graphics_pipeline_create_info.pStages             = shader_stages;
    graphics_pipeline_create_info.pVertexInputState   = &pipeline_vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
    graphics_pipeline_create_info.pViewportState      = &pipeline_viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState   = &pipeline_multisample_state_create_info;
    graphics_pipeline_create_info.pColorBlendState    = &pipeline_color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState       = &pipeline_dynamic_state_create_info;
    graphics_pipeline_create_info.layout              = pipeline_layout;
    graphics_pipeline_create_info.renderPass          = render_pass;
    graphics_pipeline_create_info.subpass             = 0;
    graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;

    VkPipeline graphics_pipeline{};
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &graphics_pipeline));

    vkDestroyShaderModule(m_Device, vert_shader_module, nullptr);
    vkDestroyShaderModule(m_Device, frag_shader_module, nullptr);

    std::vector<VkFramebuffer> swapchain_framebuffers(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++) {

        VkImageView attachments[] = { swapchain_image_views[i] };

        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass      = render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments    = attachments;
        framebuffer_create_info.width           = swapchain_extent.width;
        framebuffer_create_info.height          = swapchain_extent.height;
        framebuffer_create_info.layers          = 1;

        VK_CHECK_RESULT(vkCreateFramebuffer(m_Device, &framebuffer_create_info, nullptr, &swapchain_framebuffers[i]));
    }

    VkCommandPool           command_pool{};
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = graphics_queue_family_index;

    VK_CHECK_RESULT(vkCreateCommandPool(m_Device, &command_pool_create_info, nullptr, &command_pool));

    m_Vertices.emplace_back(Vertex{ glm::vec3(-0.5f,  0.5f, 0.0f) });
    m_Vertices.emplace_back(Vertex{ glm::vec3( 0.5f,  0.5f, 0.0f) });
    m_Vertices.emplace_back(Vertex{ glm::vec3( 0.0f, -0.5f, 0.0f) });

    m_Indices.emplace_back(0);
    m_Indices.emplace_back(1);
    m_Indices.emplace_back(2);

    this->createVertexBuffer(physical_device, graphics_queue, command_pool);
    this->createIndexBuffer(physical_device, graphics_queue, command_pool);

    constexpr int                MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkCommandBuffer> command_buffers(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo  command_buffer_allocate_info{};
    command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandBufferCount = command_buffers.size();
    command_buffer_allocate_info.commandPool        = command_pool;
    command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, command_buffers.data()));

    std::vector<VkSemaphore> image_avaliable_semaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> render_finished_semaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkFence>     in_flight_fences(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, nullptr, &image_avaliable_semaphores[i]));
        VK_CHECK_RESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, nullptr, &render_finished_semaphores[i]));
        VK_CHECK_RESULT(vkCreateFence(m_Device, &fence_create_info, nullptr, &in_flight_fences[i]));
    }

    uint32_t current_frame = 0;

    using clock    = std::chrono::high_resolution_clock;
    auto last_time = clock::now();
    int  frames    = 0;

    while (!glfwWindowShouldClose(m_GLFWwindow)) {
        glfwPollEvents();

        uint32_t image_index = 0;
        VK_CHECK_RESULT(vkAcquireNextImageKHR(m_Device, swapchain, UINT64_MAX, image_avaliable_semaphores[current_frame], VK_NULL_HANDLE, &image_index));

        VK_CHECK_RESULT(vkResetCommandBuffer(command_buffers[current_frame], 0));

        record_command_buffer(command_buffers[current_frame], image_index, render_pass, swapchain_framebuffers, swapchain_extent, graphics_pipeline);

        VkPipelineStageFlags wait_stages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore          wait_semaphores[]   = { image_avaliable_semaphores[current_frame] };
        VkSemaphore          signal_semaphores[] = { render_finished_semaphores[current_frame] };
        VkSwapchainKHR       swapchains[]        = { swapchain };

        VkSubmitInfo submit_info{};
        submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount   = 1;
        submit_info.pCommandBuffers      = &command_buffers[current_frame];
        submit_info.pWaitDstStageMask    = wait_stages;
        submit_info.waitSemaphoreCount   = 1;
        submit_info.pWaitSemaphores      = wait_semaphores;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores    = signal_semaphores;

        VK_CHECK_RESULT(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]));

        VkPresentInfoKHR present_info{};
        present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = signal_semaphores;
        present_info.swapchainCount     = 1;
        present_info.pSwapchains        = swapchains;
        present_info.pImageIndices      = &image_index;

        VK_CHECK_RESULT(vkQueuePresentKHR(present_queue, &present_info));

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

        frames++;
        auto now      = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_time);

        if (duration.count() >= 1) {
            fe::logging::info("FPS : %i", frames);
            frames    = 0;
            last_time = now;
        }
    }
}

fe::RendererVulkan::~RendererVulkan() {
}

void fe::RendererVulkan::ClearScreen(float red, float green, float blue, float alpha) {
}

void fe::RendererVulkan::SwapBuffers() {
}

void fe::RendererVulkan::Draw(MeshID index) {
}
