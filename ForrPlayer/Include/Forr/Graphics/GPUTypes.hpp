/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include "Core/string.hpp"

#include <variant>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace fe {
    //#pragma pack(push, 1) // disabled for now
    struct FORR_API Vertex {
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

    struct alignas(16) FORR_API GPULight {
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
    struct alignas(16) FORR_API GPUPBRMaterial {
        std::uint64_t base_color_texture_handle{};
    };

    using Index = uint32_t; // convert all to uint32_t ( at least for now )

    //#pragma pack(pop) // pack(push, 1) // disabled for now

    using Vertices = std::vector<Vertex>;
    using Indices  = std::vector<Index>;

    namespace render_graph {
        enum class FORR_API RenderMode : uint8_t {
            POINTS,
            LINES,
            LINE_LOOP,
            LINE_STRIP,
            TRIANGLES,
            TRIANGLE_STRIP,
            TRIANGLE_FAN,
        };

        enum class FORR_API RenderIndexType : uint8_t {
            UNSIGNED_BYTE,
            UNSIGNED_SHORT,
            UNSIGNED_INT,
        };

        enum class FORR_API ResourceState {
            UNDEFINED,

            RENDER_TARGET,

            DEPTH_WRITE,
            DEPTH_READ,

            SHADER_READ_ONLY,

            UNORDERED_ACCESS,

            COPY_SRC,
            COPY_DST
        };

        enum class FORR_API ImageType : uint8_t {
            IMAGE_TYPE_1D,
            IMAGE_TYPE_2D,
            IMAGE_TYPE_3D,
        };

        enum class FORR_API Format : uint32_t {
            UNDEFINED,

            RGBA8_UNORM,
            RGBA8_SRGB,
            BGRA8_UNORM,

            RGBA16_SFLOAT,
            R11G11B10_SFLOAT,
            RG16_SFLOAT,

            R32_UINT,
            R32_SFLOAT,

            D32_SFLOAT,
            D24_UNORM_S8_UINT,
            D32_SFLOAT_S8_UINT
        };

        enum class FORR_API ImageUsageBits : uint32_t {
            NONE             = 0,
            RENDER_TARGET    = 1 << 0,
            DEPTH_STENCIL    = 1 << 1,
            SHADER_READ      = 1 << 2,
            UNORDERED_ACCESS = 1 << 3,
            COPY_SRC         = 1 << 4,
            COPY_DST         = 1 << 5
        };

        enum class FORR_API BufferUsageBits : uint32_t {
            NONE = 0,
            // ...
        };

        struct FORR_API Rect2D {
            glm::ivec2 offset{};
            glm::ivec2 extent{};
        };

        struct FORR_API ImageDesc {
            fe::StringHash hash{};
            ImageType      type{};
            Format         format{};
            glm::ivec3     extent{};
            uint32_t       mip_levels{};
            ImageUsageBits usage{};

            ImageDesc() = default;
            ImageDesc(fe::StringHash    hash,
                      ImageType         type,
                      Format            format,
                      const glm::ivec3& extent,
                      uint32_t          mip_levels,
                      ImageUsageBits    usage)
                : hash(hash), type(type), format(format), extent(extent), mip_levels(mip_levels), usage(usage) {}
        };

        struct FORR_API ImageBarrier {
            fe::StringHash hash{};
            ResourceState  old_state{};
            ResourceState  new_state{};

            ImageBarrier() = default;
            ImageBarrier(fe::StringHash hash,
                         ResourceState  old_state,
                         ResourceState  new_state)
                : hash(hash), old_state(old_state), new_state(new_state) {}
        };

        // CreateCommands are used when RenderGraph compiles ( fe::RenderGraph::Compile() )   
        using CreateCommand     = std::variant<ImageDesc>; // TODO : add BufferDesc
        using CreateCommandList = std::vector<CreateCommand>;

        // RenderCommands are used every frame by RenderGraph ( ...TODO : add functions to work with fe::IRenderer ) 
        using RenderCommand     = std::variant<ImageBarrier>; // TODO : add BufferBarrier
        using RenderCommandList = std::vector<RenderCommand>;
    } // namespace render_graph

    namespace shader {
        enum class FORR_API DescriptorType : std::uint8_t {
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
        enum class FORR_API ValueType : std::uint8_t {
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
        struct FORR_API ReflectedDataNode {
            ValueType type{ ValueType::UNKNOWN };

            uint32_t array_size{ 1 };

            uint32_t size{};

            std::vector<ReflectedMember> members{};

            // TODO : provide attribute name
            fe::hashed_string name{};

            ReflectedDataNode() = default;
            ReflectedDataNode(ValueType type, uint32_t array_size, uint32_t size, std::vector<ReflectedMember> members, std::string name)
                : type(type), array_size(array_size), size(size), members(std::move(members)), name(std::move(name)) {}

            bool operator==(const ReflectedDataNode&) const noexcept = default;
        };

        // may be a field of a shader struct
        struct FORR_API ReflectedMember : public ReflectedDataNode {
            uint32_t offset{};

            bool operator==(const ReflectedMember&) const noexcept = default;
        };

        struct FORR_API ReflectedDescriptor : public ReflectedDataNode {
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

        struct FORR_API ReflectedPushConstants : public ReflectedDataNode {
            uint8_t stage_flags{};

            ReflectedPushConstants() = default;
            ReflectedPushConstants(ReflectedDataNode data_node, uint8_t stage_flags)
                : ReflectedDataNode(std::move(data_node)), stage_flags(stage_flags) {}

            bool operator==(const ReflectedPushConstants&) const noexcept = default;
        };

        enum class FORR_API StageBits : std::uint8_t {
            NONE     = 0,
            VERTEX   = 1 << 0,
            GEOMETRY = 1 << 1,
            FRAGMENT = 1 << 2,
            COMPUTE  = 1 << 3,
        };

        struct FORR_API ReflectedPipelineLayout {
            std::vector<shader::ReflectedDescriptor> descriptors{};
            shader::ReflectedPushConstants           push_constants{};

            ReflectedPipelineLayout() = default;
            ReflectedPipelineLayout(std::vector<shader::ReflectedDescriptor> descriptors, shader::ReflectedPushConstants push_constants)
                : descriptors(std::move(descriptors)), push_constants(std::move(push_constants)) {}

            bool operator==(const ReflectedPipelineLayout&) const noexcept = default;
        };

        struct FORR_API ReflectedMaterialLayout {
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
