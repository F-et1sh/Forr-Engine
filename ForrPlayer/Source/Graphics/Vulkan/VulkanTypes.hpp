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
        VulkanVertexBuffer vertex_buffer{};
        VulkanIndexBuffer  index_buffer{};

        uint32_t index_offset{};
        uint32_t index_count{};

        //GLenum render_mode{ GL_TRIANGLES }; // triangles by default | TODO : provide topology

        fe::pointer<resource::Material> material{};

        VulkanPrimitive()  = default;
        ~VulkanPrimitive() = default;

        FORR_CLASS_NONCOPYABLE(VulkanPrimitive)
        FORR_CLASS_MOVABLE(VulkanPrimitive)
    };

    struct VulkanMesh {
        // GLenum index_type{}; uint32_t for all models ( at least for now )

        std::vector<VulkanPrimitive> primitives{};

        VulkanMesh()  = default;
        ~VulkanMesh() = default;

        FORR_CLASS_NONCOPYABLE(VulkanMesh)
        FORR_CLASS_MOVABLE(VulkanMesh)
    };

    // for 1:1 mapping | TODO : don't do this. You don't have to create Model,
    //  Mesh and other things like this on GPU-side
    struct VulkanModel {
        std::vector<fe::pointer<VulkanMesh>>    pointers_mesh{};
        std::vector<fe::pointer<VulkanTexture>> pointers_texture{};

        VulkanModel()  = default;
        ~VulkanModel() = default;

        FORR_CLASS_NONCOPYABLE(VulkanModel)
        FORR_CLASS_MOVABLE(VulkanModel)
    };

    namespace temp { // TODO : use this instead of the structures upper
        struct VulkanPrimitive {
            uint32_t index_offset{};
            uint32_t index_count{};

            fe::pointer<resource::Material> material{};

            VulkanPrimitive()  = default;
            ~VulkanPrimitive() = default;

            FORR_CLASS_NONCOPYABLE(VulkanPrimitive)
            FORR_CLASS_MOVABLE(VulkanPrimitive)
        };

        struct VulkanMesh {
            VulkanVertexBuffer vertex_buffer{};
            VulkanIndexBuffer  index_buffer{};

            std::vector<temp::VulkanPrimitive> primitives{};

            VulkanMesh()  = default;
            ~VulkanMesh() = default;

            FORR_CLASS_NONCOPYABLE(VulkanMesh)
            FORR_CLASS_MOVABLE(VulkanMesh)
        };

        struct VulkanModel {
            std::vector<fe::pointer<temp::VulkanMesh>> pointers_mesh{};
            std::vector<fe::pointer<VulkanTexture>>    pointers_texture{};

            VulkanModel()  = default;
            ~VulkanModel() = default;

            FORR_CLASS_NONCOPYABLE(VulkanModel)
            FORR_CLASS_MOVABLE(VulkanModel)
        };
    } // namespace temp
} // namespace fe
