/*===============================================

    Forr Engine

    File : VulkanTypes.hpp
    Role : Vulkan types. All structures here must be movable only
        Even if the structure is only 4 bytes

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

    // there is no difference between 'VulkanUniformBuffer' and this structure
    struct VulkanStorageBuffer {
        fe::vk::DeviceMemory memory{};
        fe::vk::Buffer       buffer{};

        VkDescriptorSet descriptor_set{};
        uint8_t*        mapped{};

        VulkanStorageBuffer()  = default;
        ~VulkanStorageBuffer() = default;

        FORR_CLASS_NONCOPYABLE(VulkanStorageBuffer)
        FORR_CLASS_MOVABLE(VulkanStorageBuffer)
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

    template <typename T>
    struct VulkanResourceTraits;

#define VULKAN_RESOURCE_TRAITS_INSTANCE(CPU_TYPE, GPU_TYPE) \
    template <>                                             \
    struct VulkanResourceTraits<CPU_TYPE> {                 \
        using type = GPU_TYPE;                              \
    };

    VULKAN_RESOURCE_TRAITS_INSTANCE(resource::Model::Mesh, VulkanMesh)
} // namespace fe
