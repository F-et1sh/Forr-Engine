/*===============================================

    Forr Engine

    File : VulkanSwapchain.hpp
    Role : temp

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "VKTypes.hpp"
#include "VulkanContext.hpp"

namespace fe {
    class VulkanSwapchain {
    public:
        VulkanSwapchain(VulkanContext& context) : m_Context(context) {}
        ~VulkanSwapchain() = default;

    private:
        VulkanContext& m_Context;

        VkFormat        m_ColorFormat{};
        VkColorSpaceKHR m_ColorSpace{};

        std::vector<fe::vk::Image>     m_Images{};
        std::vector<fe::vk::ImageView> m_ImageViews{};

        uint32_t m_QueueNodeIndex{ ~0 };
        uint32_t m_ImageCount{};
    };
} // namespace fe
