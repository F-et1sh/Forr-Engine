/*===============================================

    Forr Engine

    File : GPUTypes.hpp
    Role : Unified GPU types for every renderer
        TODO : collapse this file to 3 separate files

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/custom_allocators.hpp"
#include "Core/pointer.hpp"
#include "Core/string.hpp"
#include "Core/logging.hpp"

#include <variant>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace fe {
    enum class GraphicsBackend {
        OpenGL,
        Vulkan
    };

    enum class PlatformBackend {
        GLFW
    };

    // TODO : move this into Core
    static constexpr void hash_combine(std::size_t& seed, std::size_t value) noexcept {
        seed ^= value + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
    };

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

    // TODO : remove
    struct alignas(16) FORR_API GPULight {
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
    struct alignas(16) FORR_API GPUPBRMaterial {
        std::uint64_t base_color_texture_handle{};
    };

    using Index = uint32_t; // convert all to uint32_t ( at least for now )

    //#pragma pack(pop) // pack(push, 1) // disabled for now

    using Vertices = std::vector<Vertex>;
    using Indices  = std::vector<Index>;

    /* forward declarations */
    namespace shader {
        struct ReflectedDescriptor;
    } // namespace shader
    namespace resource {
        struct ShaderFileData;
        struct Material;
        struct Model;
    } // namespace resource
    class ResourceManager;

    inline static constexpr size_t MAX_COLOR_ATTACHMENTS = 16;

    enum class FORR_API RenderIndexType : uint8_t {
        UNSIGNED_BYTE,
        UNSIGNED_SHORT,
        UNSIGNED_INT,
    };

    enum class FORR_API ResourceState : uint8_t {
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

    enum class FORR_API RenderMode : uint8_t {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
    };

    enum class FORR_API DepthMode : uint8_t {
        NEVER,
        LESS,
        EQUAL,
        LEQUAL,
        GREATER,
        NOTEQUAL,
        GEQUAL,
        ALWAYS
    };

    enum class FORR_API CullMode : uint8_t {
        NONE,
        FRONT,
        BACK,
        FRONT_AND_BACK
    };

    struct FORR_API PipelineFlags {
        RenderMode render_mode{ RenderMode::TRIANGLES };

        bool      depth_test_enable{ true };
        DepthMode depth_mode{ DepthMode::LESS };

        bool     cull_enable{ true };
        CullMode cull_mode{ CullMode::FRONT_AND_BACK };
    };

    struct FORR_API VertexLayout {
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
        struct FORR_API ParameterDesc {
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
            UNSUPPORTED_MEMORY_TYPE,
            MAPPED_MEMORY_WAS_NULLPTR
        };

        struct FORR_API ParameterHandleFields {
            uint8_t set{ std::numeric_limits<uint8_t>::max() };
            uint8_t binding{ std::numeric_limits<uint8_t>::max() };
        };

        struct FORR_API ParameterHandlePacker {
            // [index 4 bytes] [generation 2 bytes] [set 1 byte] [binding 1 byte] -> 8 byte together
            FORR_NODISCARD static constexpr uint64_t operator()(uint32_t index, uint16_t generation, ParameterHandleFields fields) noexcept {
                return (static_cast<uint64_t>(index) << 32) |
                       (static_cast<uint64_t>(generation) << 16) |
                       (static_cast<uint64_t>(fields.set) << 8) |
                       static_cast<uint64_t>(fields.binding);
            }
        };

        struct FORR_API ParameterHandleUnpacker {
            // 8 byte together --> [index 4 bytes] [generation 2 bytes] [set 1 byte] [binding 1 byte]
            FORR_NODISCARD static constexpr std::tuple<uint32_t, uint16_t, ParameterHandleFields> operator()(uint64_t packed) noexcept {
                uint32_t index      = static_cast<uint32_t>(packed >> 32);
                uint16_t generation = static_cast<uint16_t>((packed >> 16) & 0xFFFF);

                ParameterHandleFields fields{};
                fields.set     = static_cast<uint8_t>((packed >> 8) & 0xFFFF);
                fields.binding = static_cast<uint8_t>(packed & 0xFFFF);

                return { index, generation, fields };
            }
        };

        using ParameterHandle = fe::pointer<struct ParameterTag,    // a tag to define that this handle works with parameters
                                            uint32_t,               // index                   ( 32 bytes )
                                            uint16_t,               // generation              ( 16 bytes )
                                            uint64_t,               // packed aka all together ( 64 bytes )
                                            ParameterHandleFields>; //                         ( 16 bytes )

        struct FORR_API PipelineDesc {
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

        template <typename T>
        concept ResourceHandleSeparator =
            std::is_same_v<T, uint64_t> ||                // unified usage ( mostly inside of render graph )
            std::is_same_v<T, graphics::TextureHandle> || // when getting image barriers from render graph
            std::is_same_v<T, graphics::BufferHandle>;    // when getting buffer barriers from render graph

        // this needs for 'RenderGraph ( and user interface ) <-> GPU resource manager' connection
        template <ResourceHandleSeparator IDType>
        struct FORR_API ResourceHandle {
            // hashed name - used for user interface ( fe::string_hash("ShadowMap") )
            fe::StringHash hashed_name{};

            // index in GPU resource manager's strage - used when render passes are already compiled
            IDType storage_index { std::numeric_limits<resource_packed>::max(); };

            ResourceHandle() = default;
            ResourceHandle(fe::StringHash hashed_name) : hashed_name(hashed_name) {}
            explicit ResourceHandle(fe::StringHash hashed_name, IDType storage_index) : hashed_name(hashed_name), storage_index(storage_index) {}

            bool operator==(const ResourceHandle& other) const noexcept { return storage_index == other.storage_index; }
        };

        using ImageHandle  = ResourceHandle<graphics::TextureHandle>;
        using BufferHandle = ResourceHandle<graphics::BufferHandle>;

        // creation commands aka resource descs ( this commands must not be in 'FORR_RENDER_COMMANDS_LIST' )

        struct FORR_API ImageDesc {
            ImageHandle handle{};

            ImageType      type{};
            Format         format{};
            glm::ivec3     extent{};
            uint32_t       mip_levels{};
            ImageUsageBits usage{};

            bool operator==(const ImageDesc& other) const noexcept = default;
        };

        struct FORR_API BufferDesc {
            BufferHandle handle{};

            size_t          size_in_bytes{};
            BufferUsageBits usage{};

            bool operator==(const BufferDesc& other) const noexcept = default;
        };

        using CreationCommand     = std::variant<ImageDesc, BufferDesc>;
        using CreationCommandList = std::vector<CreationCommand>;

        // render commands ( this commands must be in 'FORR_RENDER_COMMANDS_LIST' below )

        template <ResourceHandleSeparator IDType>
        struct FORR_API ResourceBarrier {
            ResourceHandle<IDType> handle{};
            ResourceState          old_state{};
            ResourceState          new_state{};

            ResourceBarrier() = default;
            ResourceBarrier(fe::StringHash hashed_name,
                            ResourceState  old_state,
                            ResourceState  new_state)
                : handle(ResourceHandle{ hashed_name, static_cast<size_t>(~0) }), old_state(old_state), new_state(new_state) {}
        };

        using ImageBarrier  = ResourceBarrier<graphics::TextureHandle>;
        using BufferBarrier = ResourceBarrier<graphics::BufferHandle>;

        struct FORR_API BeginRenderPass {
            bool is_to_screen{};
            bool is_clears_color{};
            bool is_clears_depth{};
            bool has_depth_target{};

            Rect2D viewport{};

            glm::vec4 clear_color_value{};
            double    clear_depth_value{};

            // TODO : now I use 'std::array' here to make the structure plain data-oriented object
            //          but I would like to use 'std::vector' here
            std::array<graphics::TextureHandle, MAX_COLOR_ATTACHMENTS> color_targets{};
            size_t                                                     color_targets_count{};

            graphics::TextureHandle depth_target{};
        };

        inline static std::size_t color_depth_targets_hash(const std::array<graphics::TextureHandle, MAX_COLOR_ATTACHMENTS>& color_targets,
                                                           size_t                                                            color_targets_count,
                                                           graphics::TextureHandle                                           depth_target) {
            std::size_t seed{};

            for (size_t i = 0; i < color_targets_count; i++) {
                hash_combine(seed, std::hash<uint64_t>{}(static_cast<uint64_t>(color_targets[i])));
            }
            hash_combine(seed, std::hash<uint64_t>{}(static_cast<uint64_t>(depth_target)));

            return seed;
        }

        struct FORR_API EndRenderPass {
            // this is empty for now
        };

        struct FORR_API DrawIndexed {
            uint32_t index_count{};
            uint32_t instance_count{};
            uint32_t first_index{};
            int32_t  vertex_offset{};
            uint32_t first_instance{};
        };

        struct FORR_API BindPipeline {
            graphics::PipelineHandle pipeline_id{};
        };

        // temp
        struct FORR_API DrawModel {
            fe::pointer<resource::Model> model_ptr{};
            uint32_t                     first_instance{};

            DrawModel() = default;
            DrawModel(fe::pointer<resource::Model> model_ptr, uint32_t first_instance = {})
                : model_ptr(model_ptr), first_instance(first_instance) {}
        };

        struct FORR_API BindBuffer {
            graphics::ParameterHandle parameter_id{};
        };

        struct FORR_API WriteBuffer {
            graphics::ParameterHandle  parameter_id{};
            std::span<const std::byte> data{};

            WriteBuffer() = default;
            WriteBuffer(graphics::ParameterHandle parameter_id, std::span<const std::byte> data)
                : parameter_id(parameter_id), data(data) {}
        };

        // To add a command write its structure and add it here
// render commands - theys are used every frame by RenderGraph(fe::RenderGraph::Execute())
#define FORR_RENDER_COMMANDS_LIST(X) \
    X(ImageBarrier)                  \
    X(BufferBarrier)                 \
    X(BeginRenderPass)               \
    X(EndRenderPass)                 \
    X(DrawIndexed)                   \
    X(BindPipeline)                  \
    X(DrawModel)                     \
    X(BindBuffer)                    \
    X(WriteBuffer)

        enum class CommandType : uint8_t {
#define GENERATE_ENUM(COMMAND_NAME) COMMAND_NAME,
            FORR_RENDER_COMMANDS_LIST(GENERATE_ENUM)
#undef GENERATE_ENUM
        };

        template <typename T>
        struct CommandTraits;

#define GENERATE_TRAITS(COMMAND_NAME)                                       \
    template <>                                                             \
    struct FORR_API CommandTraits<COMMAND_NAME> {                           \
        static constexpr CommandType      Type = CommandType::COMMAND_NAME; \
        static constexpr std::string_view Name = #COMMAND_NAME;             \
    };

        FORR_RENDER_COMMANDS_LIST(GENERATE_TRAITS)
#undef GENERATE_TRAITS

        class FORR_API CommandList {
        public:
            CommandList()  = default;
            ~CommandList() = default;

            FORR_CLASS_MOVABLE(CommandList)
            FORR_CLASS_NONCOPYABLE(CommandList)

            template <typename Command>
                requires std::is_trivially_copyable_v<Command> && std::is_trivially_destructible_v<Command>
            void enqueue(const Command& command) {
                constexpr CommandType type = CommandTraits<Command>::Type;

                m_storage.emplace_back(static_cast<uint8_t>(type));
                size_t offset = fe::align_up(alignof(Command), m_storage.size());
                m_storage.resize(offset + sizeof(Command));
                new (&m_storage[offset]) Command{ command };
            }

            template <typename Func>
            void handle_all(Func&& func) {
                if (this->empty()) return;

                uint8_t*     buffer     = this->data();
                const size_t total_size = this->size();
                size_t       offset     = 0;

                while (offset < total_size) {
                    offset = fe::align_up(alignof(render_graph::CommandType), offset);
                    if (offset >= total_size) break;

                    render_graph::CommandType type = static_cast<render_graph::CommandType>(buffer[offset]);
                    offset += sizeof(render_graph::CommandType);

                    switch (type) {
#define GENERATE_CASE(COMMAND_NAME)                                                 \
    case render_graph::CommandType::COMMAND_NAME: {                                 \
        offset    = fe::align_up(alignof(render_graph::COMMAND_NAME), offset);      \
        auto* cmd = reinterpret_cast<render_graph::COMMAND_NAME*>(&buffer[offset]); \
        func(*cmd);                                                                 \
        offset += sizeof(render_graph::COMMAND_NAME);                               \
        break;                                                                      \
    }

                        FORR_RENDER_COMMANDS_LIST(GENERATE_CASE)
#undef GENERATE_CASE
                        default:
                            fe::logging::error("Failed to handle a command in fe::RendererOpenGL::EndFrame() : unknown command %i", type);
                            break;
                    }
                }
            }

            template <typename Func>
            void handle_all(Func&& func) const {
                if (this->empty()) return;

                const uint8_t* buffer     = this->data();
                const size_t   total_size = this->size();
                size_t         offset     = 0;

                while (offset < total_size) {
                    offset = fe::align_up(alignof(render_graph::CommandType), offset);
                    if (offset >= total_size) break;

                    render_graph::CommandType type = static_cast<render_graph::CommandType>(buffer[offset]);
                    offset += sizeof(render_graph::CommandType);

                    switch (type) {
#define GENERATE_CASE(COMMAND_NAME)                                                             \
    case render_graph::CommandType::COMMAND_NAME: {                                             \
        offset          = fe::align_up(alignof(render_graph::COMMAND_NAME), offset);            \
        const auto* cmd = reinterpret_cast<const render_graph::COMMAND_NAME*>(&buffer[offset]); \
        func(*cmd);                                                                             \
        offset += sizeof(render_graph::COMMAND_NAME);                                           \
        break;                                                                                  \
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
                if (command_list.empty()) return;

                size_t aligned_size = fe::align_up(alignof(std::max_align_t), m_storage.size());
                m_storage.resize(aligned_size);

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

            FORR_NODISCARD uint8_t* data() noexcept {
                return m_storage.data();
            }

            FORR_NODISCARD size_t size() const noexcept {
                return m_storage.size();
            }

        private:
            std::vector<uint8_t> m_storage;
        }; // namespace render_graph

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

            GENERIC,

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

        struct FORR_API ReflectedStructureLayout {
            uint32_t                             size{};
            std::vector<shader::ReflectedMember> members{};
            fe::hashed_string                    name{};

            ReflectedStructureLayout() = default;
            ReflectedStructureLayout(uint32_t size, std::vector<ReflectedMember> members, fe::hashed_string name)
                : size(size), members(std::move(members)), name(std::move(name)) {}

            bool operator==(const ReflectedStructureLayout&) const noexcept = default;
        };

        enum class FORR_API StageBits : std::uint8_t {
            NONE     = 0,
            VERTEX   = 1 << 0,
            GEOMETRY = 1 << 1,
            FRAGMENT = 1 << 2,
            COMPUTE  = 1 << 3,
        };

        struct FORR_API ReflectedEntryPoint {
            StageBits stage_flag{};

            std::vector<fe::hashed_string> arguments{};
            std::vector<fe::hashed_string> generic_arguments{};

            fe::hashed_string name{};

            ReflectedEntryPoint() = default;
            ReflectedEntryPoint(StageBits stage_flag, std::vector<fe::hashed_string> arguments, std::vector<fe::hashed_string> generic_arguments, fe::hashed_string name)
                : stage_flag(stage_flag), arguments(std::move(arguments)), generic_arguments(std::move(generic_arguments)), name(std::move(name)) {}

            bool operator==(const ReflectedEntryPoint&) const noexcept = default;
        };

        using SpecializationValue = std::variant<fe::hashed_string,
                                                 bool,
                                                 int32_t,
                                                 uint32_t,
                                                 int64_t,
                                                 uint64_t,
                                                 float,
                                                 double>;

        struct FORR_API SpecializationArgument {
            fe::hashed_string   name{};
            SpecializationValue value{};

            bool operator==(const SpecializationArgument&) const noexcept = default;
        };

        struct FORR_API EntryPointSpecialization {
            fe::hashed_string                   name{};
            std::vector<SpecializationArgument> arguments{};

            bool operator==(const EntryPointSpecialization&) const noexcept = default;
        };

        struct FORR_API ProgramSpecialization {
            std::vector<SpecializationArgument>   global_arguments{};
            std::vector<EntryPointSpecialization> entry_points{};

            bool operator==(const ProgramSpecialization&) const noexcept = default;
        };

        using SourceCode     = std::vector<uint8_t>;
        using ProgramSources = std::unordered_map<StageBits, SourceCode>;
    } // namespace shader
} // namespace fe

template <>
struct std::hash<fe::render_graph::ImageDesc> {
    std::size_t operator()(const fe::render_graph::ImageDesc& desc) const {
        std::size_t seed{};

        // don't use 'handle' here

        fe::hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(desc.type)));
        fe::hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(desc.format)));
        fe::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(desc.extent.x)));
        fe::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(desc.extent.y)));
        fe::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(desc.extent.z)));
        fe::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(desc.mip_levels)));
        fe::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(desc.usage)));

        return seed;
    }
};

template <>
struct std::hash<fe::render_graph::BufferDesc> {
    std::size_t operator()(const fe::render_graph::BufferDesc& desc) const {
        std::size_t seed{};

        // TODO : fill buffer desc's fields

        // don't use 'handle' here

        //fe::hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(desc.type)));

        seed = 493436543653245435;

        return seed;
    }
};
