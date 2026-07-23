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
    struct ResourceHandle {
        fe::StringHash hash{};
        uint32_t       version{};

        ResourceHandle() = default;
        ResourceHandle(fe::StringHash hash, uint32_t version) : hash(hash), version(version) {}

        bool operator==(const ResourceHandle& other) const noexcept = default;
    };
} // namespace fe

// this should be in 'std::' namespace, so it's outside of 'fe::'
template <>
struct std::hash<fe::ResourceHandle> {
    std::size_t operator()(const fe::ResourceHandle& handle) const {
        std::size_t h1 = hash<fe::StringHash>()(handle.hash);
        std::size_t h2 = hash<uint32_t>()(handle.version);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

namespace fe {
    // a proxy to gather render commands from render pass
    struct FORR_API RenderGraphContext {
    };

    // a proxy to gather setup commands from render pass
    struct FORR_API RenderGraphBuilder {
        std::vector<render_graph::ImageBarrier> reads{};
        std::vector<render_graph::ImageBarrier> writes{};
        std::vector<render_graph::ImageDesc>    create_requests{};

        render_graph::ImageDesc& createImage(const render_graph::ImageDesc& desc) {
            return create_requests.emplace_back(desc);
        }

        void readImage(fe::StringHash hash, render_graph::ResourceState to_state) {
            assert(to_state != render_graph::ResourceState::RENDER_TARGET);
            assert(to_state != render_graph::ResourceState::DEPTH_WRITE);
            assert(to_state != render_graph::ResourceState::UNORDERED_ACCESS);
            assert(to_state != render_graph::ResourceState::COPY_DST);
            reads.emplace_back(render_graph::ImageBarrier{ hash, render_graph::ResourceState::UNDEFINED, to_state });
        }

        void writeImage(fe::StringHash hash, render_graph::ResourceState to_state) {
            assert(to_state != render_graph::ResourceState::DEPTH_READ);
            assert(to_state != render_graph::ResourceState::SHADER_READ_ONLY);
            assert(to_state != render_graph::ResourceState::COPY_SRC);
            writes.emplace_back(render_graph::ImageBarrier{ hash, render_graph::ResourceState::UNDEFINED, to_state });
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

        std::span<render_graph::RenderCommandList> render_commands{};

        RenderPass()  = default;
        ~RenderPass() = default;

        FORR_CLASS_MOVABLE(RenderPass)
        FORR_CLASS_NONCOPYABLE(RenderPass)
    };

    class FORR_API RenderGraph {
    public:
        struct Node { // render pass
            struct ImageBarrier {
                ResourceHandle              handle{};
                render_graph::ResourceState old_state{};
                render_graph::ResourceState new_state{};

                ImageBarrier() = default;
                ImageBarrier(const ResourceHandle& handle, render_graph::ResourceState old_state, render_graph::ResourceState new_state)
                    : handle(handle), old_state(old_state), new_state(new_state) {}
            };

            std::vector<Node::ImageBarrier>      reads{};
            std::vector<Node::ImageBarrier>      writes{};
            std::vector<render_graph::ImageDesc> create_requests{};

            Node() = default;
            Node(std::vector<Node::ImageBarrier> reads, std::vector<Node::ImageBarrier> writes, std::vector<render_graph::ImageDesc> create_requests)
                : reads(std::move(reads)), writes(std::move(writes)), create_requests(std::move(create_requests)) {}

            FORR_CLASS_MOVABLE(Node)
            FORR_CLASS_NONCOPYABLE(Node)
        };

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
        // run Setup() for every added render pass and collect its required resources via fe::RenderGraphBuilder
        void collectRenderPasses(std::vector<Node>& render_passes_dst, std::unordered_map<fe::StringHash, uint32_t>& resource_versions_map);

        // run Kahn's algorithm
        void sortRenderSasses(std::vector<Node>& render_passes_dst);

        // run culling : remove unused render passes
        void removeUnusedRenderPasses(std::vector<Node>& render_passes_dst, std::unordered_map<fe::StringHash, uint32_t>& resource_versions_map);

    private:
        std::vector<RenderPass> m_RenderPasses{};
        fe::Arena               m_RenderPassesData{ 16 * 1024 };

        render_graph::RenderCommandList m_RenderCommands{};
    };

} // namespace fe
