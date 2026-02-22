/*===============================================

    Forr Engine

    File : RendererVulkanGL.hpp
    Role : Vulkan Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IRenderer.hpp"

#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Volk/volk.h"

#include "Graphics/GPUTypes.hpp"
#include "VKTypes.hpp"

#include "VulkanContext.hpp"

namespace fe {
    class RendererVulkan : public IRenderer {
    public:
        RendererVulkan(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index);
        ~RendererVulkan();

        void ClearScreen(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) override;
        void SwapBuffers() override;

        void                                    Draw(MeshID index) override;
        FORR_FORCE_INLINE FORR_NODISCARD MeshID CreateTriangle() override { return 0; }; // temp

    private:
        // Create Vulkan base :
        // - Initialize volk
        // - Initialize fe::vk::Instance m_Instance
        // - Initialize debug messanger
        // - Initialize VkPhysicalDevice m_PhysicalDevice
        void InitializeVulkan();

    private:
        void VKCreateInstance();
        void VKChoosePhysicalDevice();

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
            VkDebugUtilsMessageTypeFlagsEXT             message_type,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void*                                       user_data);

    private:
        RendererDesc m_Description;

        IPlatformSystem& m_PlatformSystem;
        IWindow&         m_PrimaryWindow;

        fe::vk::Instance m_Instance{};
        VkPhysicalDevice m_PhysicalDevice{};

        VulkanContext m_Context{};
    };
} // namespace fe
