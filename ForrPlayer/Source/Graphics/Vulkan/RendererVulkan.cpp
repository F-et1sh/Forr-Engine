/*===============================================

    Forr Engine

    File : RendererVulkn.cpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "RendererVulkan.hpp"

#include "Tools.hpp"

#include "Volk/volk.h"

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

    VkInstance instance{};
    VK_CHECK_RESULT(vkCreateInstance(&create_info, nullptr, &instance))
    
    volkLoadInstance(instance);

    VkSurfaceKHR surface{};
    VK_CHECK_RESULT(glfwCreateWindowSurface(instance, m_GLFWwindow, nullptr, &surface))

    uint32_t device_count = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));
    if (device_count == 0) {
        fe::logging::error("Failed to initialize Vulkan Renderer. No Vulkan devices found");
        return;
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

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
}

fe::RendererVulkan::~RendererVulkan() {
}

void fe::RendererVulkan::ClearScreen(float red, float green, float blue, float alpha) {
}

void fe::RendererVulkan::SwapBuffers() {
}

void fe::RendererVulkan::Draw(MeshID index) {
}
