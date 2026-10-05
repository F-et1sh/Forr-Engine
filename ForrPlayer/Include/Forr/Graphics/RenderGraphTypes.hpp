/*===============================================

    Forr Engine

    File : RenderGraphTypes.hpp
    Role : plain types and managers for render graph

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <span>

#include "GPUTypes.hpp"

namespace fe::render_graph {
    enum class ImageType : uint8_t {
        IMAGE_TYPE_1D,
        IMAGE_TYPE_2D,
        IMAGE_TYPE_3D,
    };

    enum class Format : uint32_t {
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

    enum class ImageUsageBits : uint32_t {
        NONE             = 0,
        RENDER_TARGET    = 1 << 0,
        DEPTH_STENCIL    = 1 << 1,
        SHADER_READ      = 1 << 2,
        UNORDERED_ACCESS = 1 << 3,
        COPY_SRC         = 1 << 4,
        COPY_DST         = 1 << 5
    };

    enum class BufferUsageBits : uint32_t {
        NONE = 0,
        // ...
    };

    struct Rect2D {
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
    struct ResourceHandle {
        // hashed name - used for user interface ( fe::string_hash("ShadowMap") )
        fe::StringHash hashed_name{};

        // index in GPU resource manager's strage - used when render passes are already compiled
        IDType storage_index{};

        ResourceHandle() = default;

        ResourceHandle(fe::StringHash hashed_name)
            requires std::is_same_v<IDType, uint64_t>
            : hashed_name(hashed_name), storage_index(std::numeric_limits<uint64_t>::max()) {}

        ResourceHandle(fe::StringHash hashed_name)
            requires(!std::is_same_v<IDType, uint64_t>)
            : hashed_name(hashed_name), storage_index{} {}

        explicit ResourceHandle(fe::StringHash hashed_name, IDType storage_index) : hashed_name(hashed_name), storage_index(storage_index) {}

        bool operator==(const ResourceHandle& other) const noexcept { return storage_index == other.storage_index; }
    };

    using ImageHandle  = ResourceHandle<graphics::TextureHandle>;
    using BufferHandle = ResourceHandle<graphics::BufferHandle>;

    // creation commands aka resource descs ( this commands must not be in 'FORR_RENDER_COMMANDS_LIST' )

    struct ImageDesc {
        ImageHandle handle{};

        ImageType      type{};
        Format         format{};
        glm::ivec3     extent{};
        uint32_t       mip_levels{};
        ImageUsageBits usage{};

        bool operator==(const ImageDesc& other) const noexcept = default;
    };

    struct BufferDesc {
        BufferHandle handle{};

        size_t          size_in_bytes{};
        BufferUsageBits usage{};

        bool operator==(const BufferDesc& other) const noexcept = default;
    };

    using CreationCommand     = std::variant<ImageDesc, BufferDesc>;
    using CreationCommandList = std::vector<CreationCommand>;

    // render commands ( this commands must be in 'FORR_RENDER_COMMANDS_LIST' below )

    template <ResourceHandleSeparator IDType>
    struct ResourceBarrier {
        ResourceHandle<IDType> handle{};
        ResourceState          old_state{};
        ResourceState          new_state{};

        ResourceBarrier() = default;
        ResourceBarrier(fe::StringHash hashed_name,
                        ResourceState  old_state,
                        ResourceState  new_state)
            : handle(ResourceHandle<IDType>{ hashed_name }), old_state(old_state), new_state(new_state) {}
    };

    using ImageBarrier  = ResourceBarrier<graphics::TextureHandle>;
    using BufferBarrier = ResourceBarrier<graphics::BufferHandle>;

    struct BeginRenderPass {
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

    struct EndRenderPass {
        // this is empty for now
    };

    struct DrawIndexed {
        uint32_t index_count{};
        uint32_t instance_count{};
        uint32_t first_index{};
        int32_t  vertex_offset{};
        uint32_t first_instance{};
    };

    struct BindPipeline {
        graphics::PipelineHandle pipeline_id{};
    };

    // temp
    struct DrawModel {
        fe::pointer<resource::Model> model_ptr{};
        uint32_t                     first_instance{};

        DrawModel() = default;
        DrawModel(fe::pointer<resource::Model> model_ptr, uint32_t first_instance = {})
            : model_ptr(model_ptr), first_instance(first_instance) {}
    };

    struct BindBuffer {
        graphics::ParameterHandle parameter_id{};
    };

    struct WriteBuffer {
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
    struct CommandTraits<COMMAND_NAME> {                           \
        static constexpr CommandType      Type = CommandType::COMMAND_NAME; \
        static constexpr std::string_view Name = #COMMAND_NAME;             \
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
    };

} // namespace fe::render_graph

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
