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

    this->InitializeVulkan();
    this->InitializeDevice();
}

fe::RendererVulkan::~RendererVulkan() {
}

void fe::RendererVulkan::ClearScreen(float red, float green, float blue, float alpha) {
}

void fe::RendererVulkan::SwapBuffers() {
}

void fe::RendererVulkan::Draw(MeshID index) {
}

void fe::RendererVulkan::InitializeVulkan() {
    VK_CHECK_RESULT(volkInitialize());

    this->VKCreateInstance();
    this->VKChoosePhysicalDevice();
}

void fe::RendererVulkan::InitializeDevice() {

    // TODO : Add enabled features adding

    this->VKSetupQueueFamilyProperties();
    this->VKSetupSupportedExtensions();

    // TODO : Add enabled extensions adding

    this->VKCreateDevice();
    this->VKCreateCommandPool();
}

void fe::RendererVulkan::VKCreateInstance() {
    VkApplicationInfo app_info{};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = m_Description.application_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "Forr Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_3;

    auto extensions = this->m_PlatformSystem.getSurfaceRequiredExtensions();

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};

    debug_utils_messenger_create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_utils_messenger_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_utils_messenger_create_info.pfnUserCallback = debugUtilsMessageCallback;

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo        = &app_info;
    instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    instance_create_info.ppEnabledExtensionNames = extensions.data();
    instance_create_info.pNext                   = &debug_utils_messenger_create_info;

    VkInstance instance{};
    VK_CHECK_RESULT(vkCreateInstance(&instance_create_info, nullptr, &instance))
    m_Instance.attach(instance);

    volkLoadInstance(m_Instance);

    // === SETUP CONTEXT ===

    m_Context.instance                    = m_Instance; // instance
    m_Context.enabled_instance_extensions = extensions; // enabled instance extensions
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

    m_Context.physical_device = m_PhysicalDevice;                                               // physical device
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Context.device_properties);              // device properties
    vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Context.device_features);                  // device features
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_Context.device_memory_properties); // device memory properties
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
            m_Context.supported_extensions.push_back(e.extensionName); // supported extensions
        }
    }
}

std::vector<VkDeviceQueueCreateInfo> fe::RendererVulkan::VKGetQueueFamilyInfos(bool use_swapchain, VkQueueFlags requested_queue_types) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};

    const float default_queue_priority{};

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

void fe::RendererVulkan::VKCreateDevice(bool use_swapchain, VkQueueFlags requested_queue_types) {
    std::vector<const char*> device_extensions(m_Context.enabled_device_extensions);
    if (use_swapchain) {
        // === SETUP CONTEXT ===
        m_Context.enabled_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        // ===
    }

    // === SETUP CONTEXT ===
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = this->VKGetQueueFamilyInfos(use_swapchain, requested_queue_types);
    // ===

    VkDeviceCreateInfo device_create_info{
        .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos    = queue_create_infos.data(),
        .pEnabledFeatures     = &m_Context.enabled_device_features
    };

    VkPhysicalDeviceFeatures2 physical_device_features2{};
    if (m_Context.device_create_next_chain) {
        physical_device_features2.sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physical_device_features2.features  = m_Context.enabled_device_features;
        physical_device_features2.pNext     = m_Context.device_create_next_chain;
        device_create_info.pEnabledFeatures = nullptr;
        device_create_info.pNext            = &physical_device_features2;
    }

    if (device_extensions.empty()) return;

    for (const char* e : device_extensions) {

        auto is_extension_supported = [&](const std::string& extension) -> bool {
            return (std::find(m_Context.supported_extensions.begin(), m_Context.supported_extensions.end(), extension) != m_Context.supported_extensions.end());
        };

        if (!is_extension_supported(e)) {
            fe::logging::error("Enabled device extension %s is not present at device level", e);
        }
    }

    device_create_info.enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();

    VkDevice device{};
    VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &device));
    m_Device.attach(device);

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
            fe::logging::info(message.c_str());
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            fe::logging::info(message.c_str());
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
