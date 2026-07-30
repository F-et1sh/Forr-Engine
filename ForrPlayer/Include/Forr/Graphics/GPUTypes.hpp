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

    enum class FORR_API ResourceState : uint8_t {
        UNDEFINED,

        RENDER_TARGET,

        DEPTH_WRITE,
        DEPTH_READ,

        SHADER_READ_ONLY,

        UNORDERED_ACCESS,

        COPY_SRC,
        COPY_DST
    };

    namespace render_graph {
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

        // commands

        struct FORR_API ImageDesc {
            union {
                // hashed name - used for user interface ( fe::string_hash("ShadowMap") )
                fe::StringHash hashed_name{};

                // texture's index in GPU resource manager's strage - used, when render passes are already compiled
                size_t texture_index;
            };

            ImageType      type{};
            Format         format{};
            glm::ivec3     extent{};
            uint32_t       mip_levels{};
            ImageUsageBits usage{};

            ImageDesc() = default;
            ImageDesc(fe::StringHash    hashed_name,
                      ImageType         type,
                      Format            format,
                      const glm::ivec3& extent,
                      uint32_t          mip_levels,
                      ImageUsageBits    usage)
                : hashed_name(hashed_name), type(type), format(format), extent(extent), mip_levels(mip_levels), usage(usage) {}

            bool operator==(const ImageDesc& other) const noexcept {
                return hashed_name == other.hashed_name &&
                       type == other.type &&
                       format == other.format &&
                       extent == other.extent &&
                       mip_levels == other.mip_levels &&
                       usage == other.usage;
            }
        };

        inline static constexpr void hash_combine(std::size_t& seed, std::size_t value) noexcept {
            seed ^= value + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
        }

        inline static std::size_t image_desc_hash(const ImageDesc& image_desc) {
            std::size_t seed{};

            // don't use 'fe::render_graph::ImageDesc::hash' here

            hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(image_desc.type)));
            hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(image_desc.format)));
            hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(image_desc.extent.x)));
            hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(image_desc.extent.y)));
            hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(image_desc.extent.z)));
            hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(image_desc.mip_levels)));
            hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(image_desc.usage)));

            return seed;
        }

        struct FORR_API ImageBarrier {

            union {
                // hashed name - used for user interface ( fe::string_hash("ShadowMap") )
                fe::StringHash hashed_name{};

                // texture's index in GPU resource manager's strage - used, when render passes are already compiled
                size_t texture_index;
            };

            ResourceState old_state{};
            ResourceState new_state{};

            ImageBarrier() = default;
            ImageBarrier(fe::StringHash hash,
                         ResourceState  old_state,
                         ResourceState  new_state)
                : hashed_name(hashed_name), old_state(old_state), new_state(new_state) {}
        };

        // To add a command write its structure and add it here
#define FORR_RENDER_COMMANDS_LIST(X)                                                              \
    /*create commands - they are used when RenderGraph compiles(fe::RenderGraph::Compile()) */    \
    X(ImageDesc, ImageDesc)                                                                       \
    /* render commands - theys are used every frame by RenderGraph(fe::RenderGraph::Execute()) */ \
    X(ImageBarrier, ImageBarrier)

        enum class CommandType : uint8_t {
#define GENERATE_ENUM(EnumName, StructName) EnumName,
            FORR_RENDER_COMMANDS_LIST(GENERATE_ENUM)
#undef GENERATE_ENUM
        };

        template <typename T>
        struct CommandTraits;

#define GENERATE_TRAITS(EnumName, StructName)                           \
    template <>                                                         \
    struct CommandTraits<StructName> {                                  \
        static constexpr CommandType      Type = CommandType::EnumName; \
        static constexpr std::string_view Name = #EnumName;             \
    };

        FORR_RENDER_COMMANDS_LIST(GENERATE_TRAITS)
#undef GENERATE_TRAITS

        class CommandList {
        public:
            CommandList()  = default;
            ~CommandList() = default;

            FORR_CLASS_MOVABLE(CommandList)
            FORR_CLASS_NONCOPYABLE(CommandList)

            template <typename Command>
                requires std::is_trivially_copyable_v<Command>
            Command& enqueue(const Command& command) {
                constexpr CommandType type = CommandTraits<Command>::Type;

                m_storage.emplace_back(static_cast<uint8_t>(type));
                size_t offset = m_storage.size();
                m_storage.resize(offset + sizeof(Command));
                new (&m_storage[offset]) Command{ command };

                return reinterpret_cast<Command&>(m_storage[offset]);
            }

            template <typename Func>
            void handle_all(Func&& func) const {
                if (this->empty()) return;

                const uint8_t* buffer     = this->data();
                const size_t   total_size = this->size();
                size_t         offset     = 0;

                while (offset < total_size) {
                    render_graph::CommandType type = static_cast<render_graph::CommandType>(buffer[offset]);
                    offset += sizeof(render_graph::CommandType);

                    switch (type) {
#define GENERATE_CASE(EnumName, StructName)                                                   \
    case render_graph::CommandType::EnumName: {                                               \
        const auto* cmd = reinterpret_cast<const render_graph::StructName*>(&buffer[offset]); \
        func(*cmd);                                                                           \
        offset += sizeof(render_graph::StructName);                                           \
        break;                                                                                \
    }

                        FORR_RENDER_COMMANDS_LIST(GENERATE_CASE)
#undef GENERATE_CASE
                        default:
                            fe::logging::error("Failed to handle a command in fe::RendererOpenGL::EndFrame() : unknown command %i", type);
                            break;
                    }
                }
            }

            void append_command_list(const CommandList& command_list) {
                m_storage.append_range(command_list.m_storage);
            }

            void clear() noexcept {
                m_storage.clear();
            }

            FORR_NODISCARD bool empty() const noexcept {
                return m_storage.empty();
            }

            void reserve(size_t bytes_count) {
                m_storage.reserve(bytes_count);
            }

            FORR_NODISCARD const uint8_t* data() const noexcept {
                return m_storage.data();
            }

            FORR_NODISCARD size_t size() const noexcept {
                return m_storage.size();
            }

        private:
            std::vector<uint8_t> m_storage;
        };

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
