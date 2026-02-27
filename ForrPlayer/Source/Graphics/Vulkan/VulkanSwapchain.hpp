/*===============================================

    Forr Engine

    File : VulkanSwapchain.hpp
    Role : Separating some objects from main logic.
        This class can edit main VulkanContext

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Platform/IWindow.hpp"
#include "VKTypes.hpp"
#include "VulkanContext.hpp"
#include "Graphics/IRenderer.hpp"

namespace fe {
    class VulkanSwapchain {
    public:
        VulkanSwapchain(const RendererDesc& renderer_description, VulkanContext& context, IWindow& primary_window)
            : m_RendererDescription(renderer_description), m_Context(context), m_PrimaryWindow(primary_window) {}
        ~VulkanSwapchain() = default;

        void CreateSurface();
        void SetupSurfaceColorFormat();
        void SetupQueueNodeIndex();
        void CreateSwapchain();

        FORR_NODISCARD const std::vector<fe::vk::ImageView>& getImageViews() const { return m_ImageViews; }

    private:
        const RendererDesc& m_RendererDescription;
        VulkanContext&      m_Context;
        IWindow&            m_PrimaryWindow;

        fe::vk::Surface   m_Surface{};
        fe::vk::Swapchain m_Swapchain{};

        VkFormat        m_ColorFormat{};
        VkColorSpaceKHR m_ColorSpace{};

        VkExtent2D m_Extent{};

        std::vector<VkImage>           m_Images{}; // not RAII, because swapchain images are destroyed by driver itself
        std::vector<fe::vk::ImageView> m_ImageViews{};

        uint32_t m_QueueNodeIndex{ UINT32_MAX }; // same as ~0
        uint32_t m_ImageCount{};
    };
} // namespace fe
