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
    struct RenderGraphContext {
        // ...
    };

    namespace render_graph {
        struct ResourceHandle {
            fe::StringHash hash{};
            uint32_t       version{};
        };

        struct ImageHandle {
            ResourceHandle handle{};
        };

        struct BufferHandle {
            ResourceHandle handle{};
        };

        struct CreateImageCommand {
            ImageHandle    handle{};
            ImageType      type{};
            Format         format{};
            glm::ivec3     extent{};
            uint32_t       mip_levels{};
            ImageUsageBits usage{};
        };

        struct CreateBufferCommand {
            BufferHandle    handle{};
            uint64_t        size{};
            BufferUsageBits usage{};
        };

        struct ImageBarrierCommand {
            ImageHandle   handle{};
            ResourceState old_state{};
            ResourceState new_state{};
        };

        struct BufferBarrierCommand {
            BufferHandle  handle{};
            ResourceState old_state{};
            ResourceState new_state{};
        };

        struct BeginRenderPassCommand {
            std::vector<ImageHandle>   color_attachments{};
            std::optional<ImageHandle> depth_attachment{};
            Rect2D                     render_area{};
        };

        struct EndRenderPassCommand {};

        struct ExecuteLambdaCommand {
            std::move_only_function<void(RenderGraphContext&)> execute_function{};
        };

        using Command = std::variant<CreateImageCommand,
                                     CreateBufferCommand,
                                     ImageBarrierCommand,
                                     BufferBarrierCommand,
                                     BeginRenderPassCommand,
                                     EndRenderPassCommand,
                                     ExecuteLambdaCommand>;

        using CommandList = std::vector<Command>;
    } // namespace render_graph

    class FORR_API RenderGraphBuilder {
    public:
        enum class SizeMode : std::uint8_t {
            ABSOLUTE_PX,
            RELATIVE_TO_SCREEN
        };

        struct ImageDesc {
            SizeMode   size_mode{ SizeMode::RELATIVE_TO_SCREEN };
            glm::vec2  scale{ 1.0f, 1.0f };
            glm::ivec2 absolute_size{ 0, 0 };

            fe::Format         format{ fe::Format::RGBA8_SRGB };
            fe::ImageUsageBits usage{ fe::ImageUsageBits::RENDER_TARGET };
        };

        struct ImageCreationRequest {
            fe::fixed_string<32> name;
            ImageDesc            desc;
        };

    public:
        RenderGraphBuilder()  = default;
        ~RenderGraphBuilder() = default;

        FORR_CLASS_MOVABLE(RenderGraphBuilder);
        FORR_CLASS_NONCOPYABLE(RenderGraphBuilder);

        render_graph::ImageHandle CreateImage(std::string_view name, const ImageDesc& desc) {

            return render_graph::ImageHandle{};
        }

    private:
        std::vector<ImageDesc>  m_ImagesToCreate{};
        std::vector<BufferDesc> m_BuffersToCreate{};

        std::vector<ImageBarrier>  m_ImageBarriers{};
        std::vector<BufferBarrier> m_BufferBarriers{};

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

        SetupFunction   setup_function{};
        ExecuteFunction execute_function{};
        DestroyFunction destroy_function{};

        RenderPass()  = default;
        ~RenderPass() = default;

        FORR_CLASS_MOVABLE(RenderPass);
        FORR_CLASS_NONCOPYABLE(RenderPass);
    };

    class FORR_API RenderGraph {
    public:
        RenderGraph() = default;
        ~RenderGraph() { this->Clear(); }

        FORR_CLASS_MOVABLE(RenderGraph);
        FORR_CLASS_NONCOPYABLE(RenderGraph);

        template <typename RenderPassData, RenderPassTraits<RenderPassData> RenderPass>
        FORR_NODISCARD RenderPassData* AddPass(std::string_view name) {
            auto& render_pass = m_RenderPasses.emplace_back();

            std::byte*      mapped_data_raw = m_RenderPassesData.allocate(sizeof(std::decay_t<RenderPassData>), alignof(RenderPassData));
            RenderPassData* mapped_data     = new (mapped_data_raw) RenderPassData();

            render_pass.name             = name;
            render_pass.mapped_data      = mapped_data;
            render_pass.setup_function   = &RenderPass::Setup;
            render_pass.execute_function = [](RenderGraphContext& context, void* ptr) { RenderPass::Execute(context, *reinterpret_cast<RenderPassData*>(ptr)); };
            render_pass.destroy_function = [](void* ptr) { std::destroy_at(reinterpret_cast<RenderPassData*>(ptr)); };

            return mapped_data;
        }

        void Compile();

        void Clear();

    private:
        std::vector<RenderPass> m_RenderPasses{};
        fe::Arena               m_RenderPassesData{ 16 * 1024 };
    };

} // namespace fe
