/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"

#include <variant>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace fe {
    //#pragma pack(push, 1) // disabled for now
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 normal{};
        glm::vec2 texture_coord{};
        //glm::u16vec4 joints{};
        //glm::vec4    weights{};
        //glm::vec4    tangent{};

        Vertex(const glm::vec3& position, const glm::vec3& normal)
            : position(position), normal(normal) {}

        Vertex()  = default;
        ~Vertex() = default;
    };

    struct alignas(16) GPULight {
        //uint32_t type{};

        //float range{};
        //float inner_cone{};
        //float outer_cone{};

        glm::vec4 position{};
        glm::vec4 direction{};
        glm::vec4 color_intensity{};
    };

    // this structure only helps to calculate offsets while loading glTF model
    // you don't have to create structures like this, if you want to create your own material
    struct alignas(16) GPUPBRMaterial {
        std::uint64_t base_color_texture_handle{};
    };

    using Index = uint32_t; // convert all to uint32_t ( at least for now )

    //#pragma pack(pop) // pack(push, 1) // disabled for now

    enum class RenderMode {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
    };

    enum class RenderIndexType {
        UNSIGNED_BYTE,
        UNSIGNED_SHORT,
        UNSIGNED_INT,
    };

    using Vertices = std::vector<Vertex>;
    using Indices  = std::vector<Index>;

    namespace shader {
        enum class DescriptorType : std::uint8_t {
            UNIFORM_BUFFER,
            STORAGE_BUFFER,

            SAMPLED_IMAGE,
            SAMPLER,
            COMBINED_IMAGE_SAMPLER,
            STORAGE_IMAGE,

            ACCELERATION_STRUCTURE,

            UNKNOWN
        };

        // clang-format off
        enum class ValueType : std::uint8_t {
            VOID,

            BOOL,

            INT32, UINT32,

            INT64, UINT64,

            FLOAT16, FLOAT32, FLOAT64,

            INT8, UINT8, INT16, UINT16,

            INT_PTR, UINT_PTR,

            FLOAT2, FLOAT3, FLOAT4,

            INT2, INT3, INT4,

            UINT2, UINT3, UINT4,

            MAT3, MAT4,

            STRUCT,

            UNKNOWN
        };
        // clang-format on

        struct ReflectedMember; // forward declaration

        // base struct for reflection
        struct ReflectedDataNode {
            ValueType type{ ValueType::UNKNOWN };

            uint32_t array_size{ 1 };

            uint32_t size{};

            std::vector<ReflectedMember> members{};

            std::string name{};

            ReflectedDataNode() = default;
            ReflectedDataNode(ValueType type, uint32_t array_size, uint32_t size, std::vector<ReflectedMember> members, std::string name)
                : type(type), array_size(array_size), size(size), members(std::move(members)), name(std::move(name)) {}

            bool operator==(const ReflectedDataNode&) const noexcept = default;
        };

        // may be a field of a shader struct
        struct ReflectedMember : public ReflectedDataNode {
            uint32_t offset{};

            bool operator==(const ReflectedMember&) const noexcept = default;
        };

        struct ReflectedDescriptor : public ReflectedDataNode {
            DescriptorType descriptor_type{ DescriptorType::UNKNOWN };

            uint32_t set{};
            uint32_t binding{};

            uint8_t stage_flags{};

            bool is_bindless{};

            ReflectedDescriptor() = default;
            ReflectedDescriptor(ReflectedDataNode data_node, DescriptorType descriptor_type, uint32_t set, uint32_t binding, uint8_t stage_flags, bool is_bindless)
                : ReflectedDataNode(std::move(data_node)), descriptor_type(descriptor_type), set(set), binding(binding), stage_flags(stage_flags), is_bindless(is_bindless) {}

            bool operator==(const ReflectedDescriptor&) const noexcept = default;
        };

        struct ReflectedPushConstants : public ReflectedDataNode {
            uint8_t stage_flags{};

            ReflectedPushConstants() = default;
            ReflectedPushConstants(ReflectedDataNode data_node, uint8_t stage_flags)
                : ReflectedDataNode(std::move(data_node)), stage_flags(stage_flags) {}

            bool operator==(const ReflectedPushConstants&) const noexcept = default;
        };

        enum class StageBits : std::uint8_t {
            NONE     = 0,
            VERTEX   = 1 << 0, // 1
            GEOMETRY = 1 << 1, // 2
            FRAGMENT = 1 << 2, // 4
            COMPUTE  = 1 << 3, // 8
        };

        struct ReflectedPipelineLayout {
            std::vector<shader::ReflectedDescriptor> descriptors{};
            shader::ReflectedPushConstants           push_constants{};

            ReflectedPipelineLayout() = default;
            ReflectedPipelineLayout(std::vector<shader::ReflectedDescriptor> descriptors, shader::ReflectedPushConstants push_constants)
                : descriptors(std::move(descriptors)), push_constants(std::move(push_constants)) {}

            bool operator==(const ReflectedPipelineLayout&) const noexcept = default;
        };

        struct ReflectedMaterialLayout {
            uint32_t                             size{};
            std::vector<shader::ReflectedMember> members{};
            std::string                          name{};

            ReflectedMaterialLayout() = default;
            ReflectedMaterialLayout(uint32_t size, std::vector<ReflectedMember> members, std::string name)
                : size(size), members(std::move(members)), name(std::move(name)) {}

            bool operator==(const ReflectedMaterialLayout&) const noexcept = default;
        };
    } // namespace shader
} // namespace fe
