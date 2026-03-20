/*===============================================

    Forr Engine

    File : VulkanTypes.hpp
    Role : Vulkan types. There mustn't be complex thing. Just thin POD structures.
        This engine is mostly data-oriented designed

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "Core/pointer.hpp"
#include "VulkanRAII.hpp"
#include "ResourceManagement/Resources.hpp"

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

    struct VulkanTexture { // TODO : provide textures

        VulkanTexture()  = default;
        ~VulkanTexture() = default;

        FORR_CLASS_NONCOPYABLE(VulkanTexture)
        FORR_CLASS_MOVABLE(VulkanTexture)
    };

    struct VulkanPrimitive {
        uint32_t index_offset{};
        uint32_t index_count{};

        fe::pointer<resource::Material> material_ptr{};

        VulkanPrimitive()  = default;
        ~VulkanPrimitive() = default;
    };

    struct VulkanMesh {
        VulkanVertexBuffer vertex_buffer{};
        VulkanIndexBuffer  index_buffer{};

        std::vector<VulkanPrimitive> primitives{};

        VulkanMesh()  = default;
        ~VulkanMesh() = default;

        FORR_CLASS_NONCOPYABLE(VulkanMesh)
        FORR_CLASS_MOVABLE(VulkanMesh)
    };

    struct VulkanModel {
        std::vector<fe::pointer<VulkanMesh>>    pointers_mesh{};
        std::vector<fe::pointer<VulkanTexture>> pointers_texture{};

        VulkanModel()  = default;
        ~VulkanModel() = default;

        FORR_CLASS_NONCOPYABLE(VulkanModel)
        FORR_CLASS_MOVABLE(VulkanModel)
    };
} // namespace fe
