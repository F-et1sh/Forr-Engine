/*===============================================

    Forr Engine

    File : VulkanTypes.hpp
    Role : Vulkan types. There mustn't be complex thing. Just thin POD structures.
        This engine is mostly data-oriented designed

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "VulkanRAII.hpp"

namespace fe {
    struct VulkanImage {
        fe::vk::Image        image{};
        fe::vk::DeviceMemory memory{};
        fe::vk::ImageView    image_view{};

        VulkanImage()  = default;
        ~VulkanImage() = default;

        FORR_CLASS_NONCOPYABLE(VulkanImage)
        FORR_CLASS_MOVABLE(VulkanImage)
    };

    struct VulkanVertexBuffer {
        fe::vk::DeviceMemory memory{};
        fe::vk::Buffer       buffer{};

        VulkanVertexBuffer()  = default;
        ~VulkanVertexBuffer() = default;

        FORR_CLASS_NONCOPYABLE(VulkanVertexBuffer)
        FORR_CLASS_MOVABLE(VulkanVertexBuffer)
    };

    struct VulkanIndexBuffer {
        fe::vk::DeviceMemory memory{};
        fe::vk::Buffer       buffer{};
        size_t               count{};

        VulkanIndexBuffer()  = default;
        ~VulkanIndexBuffer() = default;

        FORR_CLASS_NONCOPYABLE(VulkanIndexBuffer)
        FORR_CLASS_MOVABLE(VulkanIndexBuffer)
    };

    struct VulkanMesh {
        VulkanVertexBuffer vertex_buffer{};
        VulkanIndexBuffer  index_buffer{};

        VulkanMesh()  = default;
        ~VulkanMesh() = default;

        FORR_CLASS_NONCOPYABLE(VulkanMesh)
        FORR_CLASS_MOVABLE(VulkanMesh)
    };

    struct VulkanUniformBuffer {
        fe::vk::DeviceMemory memory{};
        fe::vk::Buffer       buffer{};

        VkDescriptorSet descriptor_set{};
        uint8_t*        mapped{};

        VulkanUniformBuffer()  = default;
        ~VulkanUniformBuffer() = default;

        FORR_CLASS_NONCOPYABLE(VulkanUniformBuffer)
        FORR_CLASS_MOVABLE(VulkanUniformBuffer)
    };
} // namespace fe
