/*===============================================

    Forr Engine

    File : RenderGraph.hpp
    Role : render graph and an interface for render passes

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "GPUTypes.hpp"

namespace fe {
    struct ImageID {
        fe::StringHash hash{};
    };
    struct BufferID {
        fe::StringHash hash{};
    };

    struct CreateImageCommand {
        ImageID        id{};
        ImageType      type{};
        Format         format{};
        glm::ivec3     extent{};
        uint32_t       mip_levels{};
        ImageUsageBits usage{};

        CreateImageCommand()  = default;
        ~CreateImageCommand() = default;
    };

    struct CreateBufferCommand {
        BufferID        id{};
        uint64_t        size{};
        BufferUsageBits usage{};

        CreateBufferCommand()  = default;
        ~CreateBufferCommand() = default;
    };

    struct ImageBarrierCommand {
        ImageID       id{};
        ResourceState old_state{};
        ResourceState new_state{};

        ImageBarrierCommand()  = default;
        ~ImageBarrierCommand() = default;
    };

    struct BufferBarrierCommand {
        BufferID      id{};
        ResourceState old_state{};
        ResourceState new_state{};

        BufferBarrierCommand()  = default;
        ~BufferBarrierCommand() = default;
    };

    struct BeginRenderPassCommand {
        std::vector<ImageID>   color_attachments{};
        std::optional<ImageID> depth_attachment{};
        Rect2D                 render_area{};

        BeginRenderPassCommand()  = default;
        ~BeginRenderPassCommand() = default;
    };

    struct EndRenderPassCommand {};

    struct RenderGraphContext {
        // ...
    };

    struct ExecuteLambdaCommand {
        std::move_only_function<void(RenderGraphContext&)> execute_function{};
    };

    using RenderGraphCommand = std::variant<CreateImageCommand,
                                            CreateBufferCommand,
                                            ImageBarrierCommand,
                                            BufferBarrierCommand,
                                            BeginRenderPassCommand,
                                            EndRenderPassCommand,
                                            ExecuteLambdaCommand>;

    using RenderGraphCommandList = std::vector<RenderGraphCommand>;

    class RenderGraphBuilder {
    private:
        struct ResourceUsageInfo {
            fe::StringHash texture_name{};
            ResourceState  expected_state{};
        };

    public:
        RenderGraphBuilder()  = default;
        ~RenderGraphBuilder() = default;

        FORR_CLASS_MOVABLE(RenderGraphBuilder);
        FORR_CLASS_NONCOPYABLE(RenderGraphBuilder);

        void ReadTexture(fe::StringHash texture_name) {
            m_Reads.emplace_back(ResourceUsageInfo{ texture_name, ResourceState::SHADER_READ });
        }

        void WriteTexture(fe::StringHash texture_name) {
            m_Writes.emplace_back(ResourceUsageInfo{ texture_name, ResourceState::RENDER_TARGET });
        }

    private:
        std::vector<ResourceUsageInfo> m_Reads;
        std::vector<ResourceUsageInfo> m_Writes;

        friend class RenderGraph;
    };

    template <typename T, typename DataT>
    concept RenderPassTraits = requires(RenderGraphBuilder& builder, RenderGraphContext& context, DataT& render_pass_data) {
        { T::Setup(builder) } -> std::same_as<void>;
        { T::Execute(context, render_pass_data) } -> std::same_as<void>;
    };

    struct RenderPass {
        using SetupFunction   = void (*)(RenderGraphBuilder& builder);
        using ExecuteFunction = void (*)(RenderGraphContext& context, void*);
        using DestroyFunction = void (*)(void*);

        fe::fixed_string<32> name{};
        void*                mapped_data{};
        SetupFunction        setup_function{};
        ExecuteFunction      execute_function{};
        DestroyFunction      destroy_function{};

        template <typename RenderPassData, RenderPassTraits<RenderPassData> RenderPassT>
        void bind(std::string_view name, RenderPassData* mapped_data) {
            this->name        = name;
            this->mapped_data = mapped_data;

            setup_function   = &RenderPassT::Setup;
            execute_function = [](RenderGraphContext& context, void* ptr) { RenderPassT::Execute(context, *reinterpret_cast<RenderPassData*>(ptr)); };
            destroy_function = [](void* ptr) { std::destroy_at(reinterpret_cast<RenderPassData*>(ptr)); };
        }
    };

    class RenderGraph {
    public:
        RenderGraph() = default;
        ~RenderGraph() { this->Clear(); }

        FORR_CLASS_MOVABLE(RenderGraph);
        FORR_CLASS_NONCOPYABLE(RenderGraph);

        template <typename RenderPassData, RenderPassTraits<RenderPassData> RenderPassT>
        FORR_NODISCARD RenderPassData* AddPass(std::string_view name) {
            auto& render_pass = m_RenderPasses.emplace_back();

            std::byte*      mapped_data_raw = m_RenderPassesData.allocate(sizeof(std::decay_t<RenderPassData>), alignof(RenderPassData));
            RenderPassData* mapped_data     = new (mapped_data_raw) RenderPassData();

            render_pass.bind<RenderPassData, RenderPassT>(name, mapped_data);

            return mapped_data;
        }

        void Clear() {
            for (auto& pass : m_RenderPasses) {
                if (pass.destroy_function && pass.mapped_data) {
                    pass.destroy_function(pass.mapped_data);
                }
            }
            m_RenderPasses.clear();
            m_RenderPassesData.reset();
        }

    private:
        std::vector<RenderPass> m_RenderPasses{};
        fe::Arena               m_RenderPassesData{ 16 * 1024 };
    };

    struct ForwardPassData {
        float some_data{};
    };

    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder) {
            builder.WriteTexture(fe::string_hash("Color"));
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

    static void Example() {
        RenderGraph render_graph{};

        ForwardPassData* mapped_data = render_graph.AddPass<ForwardPassData, ForwardPass>("ForwardPass");
        mapped_data->some_data       = 4;
    }

} // namespace fe
