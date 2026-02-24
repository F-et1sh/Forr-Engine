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
    this->InitializeSwapchain();
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

    this->VKCreateSurface();
    this->VKSetupSurfaceColorFormat();
    this->VKSetupQueueNodeIndex();
    this->VKCreateSwapchain();
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

    auto                enabled_instance_extensions_copy = m_Context.enabled_instance_extensions; // copy
    std::vector<size_t> extensions_to_remove{};

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
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos = this->VKGetQueueFamilyInfos(use_swapchain, requested_queue_types);
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

void fe::RendererVulkan::VKCreateSurface() {
    m_Swapchain.CreateSurface();
}

void fe::RendererVulkan::VKSetupSurfaceColorFormat() {
    m_Swapchain.SetupSurfaceColorFormat();
}

void fe::RendererVulkan::VKSetupQueueNodeIndex() {
    m_Swapchain.SetupQueueNodeIndex();
}

void fe::RendererVulkan::VKCreateSwapchain() {
    m_Swapchain.CreateSwapchain();
}

std::vector<VkDeviceQueueCreateInfo> fe::RendererVulkan::VKGetQueueFamilyInfos(bool use_swapchain, VkQueueFlags requested_queue_types) {
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
