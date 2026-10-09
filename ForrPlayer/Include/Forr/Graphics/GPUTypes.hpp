/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/custom_allocators.hpp"
#include "Core/pointer.hpp"
#include "Core/string.hpp"
#include "Core/logging.hpp"

#include "ShaderReflection.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace fe {
    enum class GraphicsBackend {
        OpenGL,
        Vulkan
    };

    // TODO : move this into Core
    inline static constexpr void hash_combine(std::size_t& seed, std::size_t value) noexcept {
        seed ^= value + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
    };

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

    // TODO : remove
    struct alignas(16) GPULight {
        //uint32_t type{};

        //float range{};
        //float inner_cone{};
        //float outer_cone{};

        glm::vec4 position{};
        glm::vec4 direction{};
        glm::vec4 color_intensity{};
    };

    // TODO : remove
    // this structure only helps to calculate offsets while loading glTF model
    // you don't have to create structures like this, if you want to create your own material
    struct alignas(16) GPUPBRMaterial {
        std::uint64_t base_color_texture_handle{};
    };

    using Index = uint32_t; // convert all to uint32_t ( at least for now )

    //#pragma pack(pop) // pack(push, 1) // disabled for now

    using Vertices = std::vector<Vertex>;
    using Indices  = std::vector<Index>;

    namespace resource {
        struct ShaderFileData;
        struct Material;
        struct Model;
    } // namespace resource
    class ResourceManager;

    inline static constexpr size_t MAX_COLOR_ATTACHMENTS = 16;

    enum class RenderIndexType : uint8_t {
        UNSIGNED_BYTE,
        UNSIGNED_SHORT,
        UNSIGNED_INT,
    };

    enum class ResourceState : uint8_t {
        UNDEFINED,

        // image only
        RENDER_TARGET,
        DEPTH_WRITE,
        DEPTH_READ,

        // buffer only
        VERTEX_BUFFER,
        INDEX_BUFFER,
        CONSTANT_BUFFER,
        INDIRECT_ARGUMENT,

        // universal
        SHADER_READ_ONLY,
        UNORDERED_ACCESS,
        COPY_SRC,
        COPY_DST
    };

    enum class RenderMode : uint8_t {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
    };

    enum class DepthMode : uint8_t {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS
    };

    enum class CullMode : uint8_t {
        NONE,
        FRONT,
        BACK,
        FRONT_AND_BACK
    };

    struct PipelineFlags {
        RenderMode render_mode{ RenderMode::TRIANGLES };

        bool      depth_test_enable{ true };
        DepthMode depth_mode{ DepthMode::LESS };

        bool     cull_enable{ true };
        CullMode cull_mode{ CullMode::FRONT_AND_BACK };
    };

    struct VertexLayout {
        // TODO : there is nothing yet. I don't know what to do with this.
        // There are a few options :
        // - create some 'unified' vertex layout - I don't like this idea
        // - somehow switch vertex layout at runtime - I don't know how to do this
        // - ...
        // For now I'm just leaving this hardcoded
    };

    // forward declaration
    namespace render_graph {
        // 'resource' in render graph is an image or buffer
        // and they should be separated but able to be unified via packing the 'fe::pointer<>'
        using resource_handle     = uint32_t;
        using resource_generation = uint32_t;
        using resource_packed     = uint64_t;
    } // namespace render_graph

    namespace graphics {
        struct ParameterDesc {
            shader::DescriptorType descriptor_type{ shader::DescriptorType::UNKNOWN };
            uint8_t                stage_flags{};
            bool                   is_bindless{};
            uint32_t               array_size{ 1 };
            uint32_t               size{};
            uint32_t               set{};
            uint32_t               binding{};

            ParameterDesc() = default;
            ParameterDesc(const shader::ReflectedDescriptor& descriptor_layout)
                : descriptor_type(descriptor_layout.descriptor_type),
                  stage_flags(descriptor_layout.stage_flags),
                  is_bindless(descriptor_layout.is_bindless),
                  array_size(descriptor_layout.array_size),
                  size(descriptor_layout.size),
                  set(descriptor_layout.set),
                  binding(descriptor_layout.binding) {}
        };

        enum class ParameterCreationErrors {
            FORGOT_TO_SPECIALIZE_GENERIC_DESCRIPTOR,
            UNSUPPORTED_DESCRIPTOR_TYPE,
            MAPPED_MEMORY_WAS_NULLPTR
        };

        struct ParameterHandleFields {
            uint8_t set{ std::numeric_limits<uint8_t>::max() };
            uint8_t binding{ std::numeric_limits<uint8_t>::max() };

            FORR_NODISCARD static constexpr ParameterHandleFields operator()(const ParameterDesc& desc) noexcept {
                return { static_cast<uint8_t>(desc.set),
                         static_cast<uint8_t>(desc.binding) };
            }
        };

        struct ParameterHandlePacker {
            // [index 4 bytes] [generation 2 bytes] [set 1 byte] [binding 1 byte] -> 8 byte together
            FORR_NODISCARD static constexpr uint64_t operator()(uint32_t index, uint16_t generation, ParameterHandleFields fields) noexcept {
                return (static_cast<uint64_t>(index) << 32) |
                       (static_cast<uint64_t>(generation) << 16) |
                       (static_cast<uint64_t>(fields.set) << 8) |
                       static_cast<uint64_t>(fields.binding);
            }
        };

        struct ParameterHandleUnpacker {
            // 8 byte together --> [index 4 bytes] [generation 2 bytes] [set 1 byte] [binding 1 byte]
            FORR_NODISCARD static constexpr std::tuple<uint32_t, uint16_t, ParameterHandleFields> operator()(uint64_t packed) noexcept {
                uint32_t index      = static_cast<uint32_t>(packed >> 32);
                uint16_t generation = static_cast<uint16_t>((packed >> 16) & 0xFFFF);

                ParameterHandleFields fields{};
                fields.set     = static_cast<uint8_t>((packed >> 8) & 0xFF);
                fields.binding = static_cast<uint8_t>(packed & 0xFF);

                return { index, generation, fields };
            }
        };

        using ParameterHandle = fe::pointer<struct ParameterTag,    // a tag to define that this handle works with parameters
                                            uint32_t,               // index                   ( 32 bytes )
                                            uint16_t,               // generation              ( 16 bytes )
                                            uint64_t,               // packed aka all together ( 64 bytes )
                                            ParameterHandleFields>; //                         ( 16 bytes )

        struct PipelineDesc {
            fe::PipelineFlags pipeline_flags{};

            std::vector<fe::pointer<resource::ShaderFileData>> shader_file_data_ptrs{};

            std::vector<fe::hashed_string>   entry_points{};
            std::vector<fe::hashed_string>   descriptor_sets{};
            std::optional<fe::hashed_string> push_constants{};

            std::optional<shader::ProgramSpecialization> specialization{};
        };

        enum class PipelineCreationErrors {
            ERROR,
            // TODO : fill this up
        };

        using PipelineHandle = fe::pointer<struct PipelineTag>; // a tag to define that this handle works with pipelines

        // this handle is shared between imported assets and render graph images
        using TextureHandle = fe::pointer<struct TextureTag,
                                          render_graph::resource_handle,
                                          render_graph::resource_generation,
                                          render_graph::resource_packed>;
        // this handle is used only by render graph
        using BufferHandle = fe::pointer<struct BufferTag,
                                         render_graph::resource_handle,
                                         render_graph::resource_generation,
                                         render_graph::resource_packed>;

        using MeshHandle = fe::pointer<struct MeshTag>;
    } // namespace graphics
} // namespace fe
