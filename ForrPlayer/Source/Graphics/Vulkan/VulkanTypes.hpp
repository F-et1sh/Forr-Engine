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

    struct VulkanMaterial {
        fe::vk::DescriptorSetLayout descriptor_set_layout{};
        fe::vk::PipelineLayout      pipeline_layout{};
        fe::vk::Pipeline            pipeline{};

        VulkanMaterial()  = default;
        ~VulkanMaterial() = default;

        FORR_CLASS_NONCOPYABLE(VulkanMaterial)
        FORR_CLASS_MOVABLE(VulkanMaterial)
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

    struct VulkanShaderBuffer {
        struct Binding {
            fe::vk::DeviceMemory memory{};
            fe::vk::Buffer       buffer{};
            uint8_t*             mapped{};

            Binding()  = default;
            ~Binding() = default;

            FORR_CLASS_NONCOPYABLE(Binding)
            FORR_CLASS_MOVABLE(Binding)
        };

        VkDescriptorSet      descriptor_set{};
        std::vector<Binding> bindings{};

        VulkanShaderBuffer()  = default;
        ~VulkanShaderBuffer() = default;

        FORR_CLASS_NONCOPYABLE(VulkanShaderBuffer)
        FORR_CLASS_MOVABLE(VulkanShaderBuffer)
    };

    struct VulkanTexture {
        VkImageLayout image_layout{};

        uint32_t width{};
        uint32_t height{};
        uint32_t mip_levels{};

        vk::DeviceMemory device_memory{};
        vk::Sampler      sampler{};
        vk::Image        image{};
        vk::ImageView    view{};

        VulkanTexture()  = default;
        ~VulkanTexture() = default;

        FORR_CLASS_NONCOPYABLE(VulkanTexture)
        FORR_CLASS_MOVABLE(VulkanTexture)
    };

    struct VulkanMesh {
        VulkanVertexBuffer vertex_buffer{};
        VulkanIndexBuffer  index_buffer{};

        VulkanMesh()  = default;
        ~VulkanMesh() = default;

        FORR_CLASS_NONCOPYABLE(VulkanMesh)
        FORR_CLASS_MOVABLE(VulkanMesh)
    };
} // namespace fe
