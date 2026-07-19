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
    // a proxy to gather render commands from render pass
    struct FORR_API RenderGraphContext {
    };

    // a proxy to gather setup commands from render pass
    struct FORR_API RenderGraphBuilder {
        struct FORR_API ImageDesc {
            fe::StringHash hash{};
            ImageType      type{};
            Format         format{};
            glm::ivec3     extent{};
            uint32_t       mip_levels{};
            ImageUsageBits usage{};
        };

        struct FORR_API ImageBarrier {
            fe::StringHash hash{};
            ResourceState  to_state{};
        };

        std::vector<ImageBarrier> reads{};
        std::vector<ImageBarrier> writes{};
        std::vector<ImageDesc>    create_requests{};

        ImageDesc& createImage(const ImageDesc& desc) {
            return create_requests.emplace_back(desc);
        }

        void readImage(fe::StringHash hash, ResourceState to_state) {
            assert(to_state != ResourceState::RENDER_TARGET);
            assert(to_state != ResourceState::DEPTH_WRITE);
            assert(to_state != ResourceState::UNORDERED_ACCESS);
            assert(to_state != ResourceState::COPY_DST);
            reads.emplace_back(ImageBarrier{ hash, to_state });
        }

        void writeImage(fe::StringHash hash, ResourceState to_state) {
            assert(to_state != ResourceState::DEPTH_READ);
            assert(to_state != ResourceState::SHADER_READ_ONLY);
            assert(to_state != ResourceState::COPY_SRC);
            writes.emplace_back(ImageBarrier{ hash, to_state });
        }
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

        FORR_CLASS_MOVABLE(RenderPass)
        FORR_CLASS_NONCOPYABLE(RenderPass)
    };

    class FORR_API RenderGraph {
    public:
        struct FORR_API ImageDesc {
            ImageType      type{};
            Format         format{};
            glm::ivec3     extent{};
            uint32_t       mip_levels{};
            ImageUsageBits usage{};
        };

        struct FORR_API ImageBarrier {
            fe::StringHash hash{};
            ResourceState  old_state{};
            ResourceState  new_state{};
        };

        using GPURequestCommand = std::variant<ImageDesc, ImageBarrier>;

    public:
        RenderGraph() = default;
        ~RenderGraph() { this->Clear(); }

        FORR_CLASS_MOVABLE(RenderGraph)
        FORR_CLASS_NONCOPYABLE(RenderGraph)

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

        std::vector<GPURequestCommand> m_RequestCommands{};
    };

} // namespace fe
