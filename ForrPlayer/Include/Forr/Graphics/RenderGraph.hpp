/*===============================================

    Forr Engine

    File : RenderGraph.hpp
    Role : render graph, interface for render passes and other helpers

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "GPUTypes.hpp"

#include "entt/entt.hpp"

namespace fe {
    // this structure is needed to collect only necessary components ( for example, MeshComponent ),
    //  ignoring others. And after collecting them, continue ECS calculation for frame N+1, while renderer
    //  is working on frame N
    template <typename... Components>
    struct FORR_API RenderGraphCollector {
    private:
        entt::registry render_registry{};

    public:
        RenderGraphCollector(entt::registry& source_registry) {
            (copyComponentPool<Components>(source_registry), ...);
        }
        ~RenderGraphCollector() = default;

        FORR_CLASS_MOVABLE(RenderGraphCollector)
        FORR_CLASS_NONCOPYABLE(RenderGraphCollector)

        const entt::registry& getRegistry() const { return render_registry; }

    private:
        template <typename T>
        void copyComponentPool(entt::registry& source_registry) {
            auto view = source_registry.view<T>();

            for (auto entity : view) {
                if (!render_registry.valid(entity)) {
                    render_registry.create(entity);
                }

                render_registry.emplace_or_replace<T>(entity, view.get<T>(entity));
            }
        }
    };

    // a proxy to gather render commands from render pass
    struct FORR_API RenderGraphContext {
        render_graph::CommandList command_list{};
        const entt::registry&     render_registry{};

        // TODO :
        //void drawIndices(...) { command_list.enqueue(...) }

        RenderGraphContext(const entt::registry& render_registry) : render_registry(render_registry) {}
        ~RenderGraphContext() = default;

        FORR_CLASS_MOVABLE(RenderGraphContext)
        FORR_CLASS_NONCOPYABLE(RenderGraphContext)
    };

    // a proxy to gather setup commands from render pass
    struct FORR_API RenderGraphBuilder {
        // fe::RenderGraphBuilder is used for building RenderGraph, that's why there is no 'render_graph::CommandList'
        //  instead, all comands are separated
        std::vector<render_graph::ImageBarrier> reads{};
        std::vector<render_graph::ImageBarrier> writes{};
        std::vector<render_graph::ImageDesc>    create_requests{};

        render_graph::ImageDesc& createImage(const render_graph::ImageDesc& desc) {
            return create_requests.emplace_back(desc);
        }

        void readImage(fe::StringHash hash, ResourceState to_state) {
            assert(to_state != ResourceState::RENDER_TARGET);
            assert(to_state != ResourceState::DEPTH_WRITE);
            assert(to_state != ResourceState::UNORDERED_ACCESS);
            assert(to_state != ResourceState::COPY_DST);
            reads.emplace_back(render_graph::ImageBarrier{ hash, ResourceState::UNDEFINED, to_state });
        }

        void writeImage(fe::StringHash hash, ResourceState to_state) {
            assert(to_state != ResourceState::DEPTH_READ);
            assert(to_state != ResourceState::SHADER_READ_ONLY);
            assert(to_state != ResourceState::COPY_SRC);
            writes.emplace_back(render_graph::ImageBarrier{ hash, ResourceState::UNDEFINED, to_state });
        }

        RenderGraphBuilder()  = default;
        ~RenderGraphBuilder() = default;

        FORR_CLASS_MOVABLE(RenderGraphBuilder)
        FORR_CLASS_NONCOPYABLE(RenderGraphBuilder)
    };

    // this structure is needed for the 'fe::RenderGraph::Compile()'
    //  it is used by the renderer to create all resources
    struct FORR_API RenderGraphCompileResult {
        std::vector<render_graph::ImageDesc> image_descs{}; // create requests

        RenderGraphCompileResult()  = default;
        ~RenderGraphCompileResult() = default;

        FORR_CLASS_MOVABLE(RenderGraphCompileResult)
        FORR_CLASS_NONCOPYABLE(RenderGraphCompileResult)
    };

    // this structure is needed for the 'fe::RenderGraph::SetupResourceBindings()'
    struct FORR_API RenderGraphBindings {
        // virtual storage index --> GPU storage index
        std::unordered_map<size_t, size_t> bindings{};

        RenderGraphBindings()  = default;
        ~RenderGraphBindings() = default;

        FORR_CLASS_MOVABLE(RenderGraphBindings)
        FORR_CLASS_NONCOPYABLE(RenderGraphBindings)
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

        std::vector<render_graph::ImageBarrier> compiled_barriers{};

        RenderPass()  = default;
        ~RenderPass() = default;

        FORR_CLASS_MOVABLE(RenderPass)
        FORR_CLASS_NONCOPYABLE(RenderPass)
    };

    // this is the structure you get in 'fe::RenderGraph::AddPass<>' to
    //  access your mapped data and render pass' index in the array
    template <typename RenderPassData>
    struct RenderPassHandle {
        RenderPassData* mapped_data{};
        // render pass' index in 'fe::RenderGraph::m_RenderPasses'
        uint32_t render_pass_index{};

        RenderPassHandle()  = default;
        ~RenderPassHandle() = default;

        FORR_CLASS_MOVABLE(RenderPassHandle)
        FORR_CLASS_NONCOPYABLE(RenderPassHandle)
    };

    class FORR_API RenderGraph {
    public:
        // this structure is used to access resources in a map
        // different versions of the same resource cannot be accessed
        struct ResourceHandle {
            fe::StringHash hashed_name{};
            uint32_t       version{};

            ResourceHandle() = default;
            ResourceHandle(fe::StringHash hashed_name, uint32_t version) : hashed_name(hashed_name), version(version) {}

            bool operator==(const ResourceHandle& other) const noexcept = default;
        };

        // this structure is used to hold version and the previous state of the resource
        struct Resource {
            uint32_t      version{};
            ResourceState old_state{ ResourceState::UNDEFINED };

            Resource() = default;
            Resource(uint32_t version, ResourceState old_state) : version(version), old_state(old_state) {}

            bool operator==(const Resource& other) const noexcept = default;
        };

        struct ResourceLifetime {
            uint32_t first_pass_index = std::numeric_limits<uint32_t>::max();
            uint32_t last_pass_index  = 0;

            ResourceLifetime() = default;
            ResourceLifetime(uint32_t first_pass_index, uint32_t last_pass_index)
                : first_pass_index(first_pass_index), last_pass_index(last_pass_index) {}

            bool operator==(const ResourceLifetime& other) const noexcept = default;
        };

        struct CompiledRenderPass { // a node for sorting algorithm
            struct ImageBarrier {
                ResourceHandle handle{};
                ResourceState  old_state{};
                ResourceState  new_state{};

                ImageBarrier() = default;
                ImageBarrier(const ResourceHandle& handle, ResourceState old_state, ResourceState new_state)
                    : handle(handle), old_state(old_state), new_state(new_state) {}
            };

            fe::fixed_string<32> name{};

            std::vector<CompiledRenderPass::ImageBarrier> reads{};
            std::vector<CompiledRenderPass::ImageBarrier> writes{};
            std::vector<render_graph::ImageDesc>          create_requests{};

            CompiledRenderPass() = default;
            CompiledRenderPass(std::string_view                              name,
                               std::vector<CompiledRenderPass::ImageBarrier> reads,
                               std::vector<CompiledRenderPass::ImageBarrier> writes,
                               std::vector<render_graph::ImageDesc>          create_requests)
                : name(name), reads(std::move(reads)), writes(std::move(writes)), create_requests(std::move(create_requests)) {}

            FORR_CLASS_MOVABLE(CompiledRenderPass)
            FORR_CLASS_NONCOPYABLE(CompiledRenderPass)
        };

    public:
        RenderGraph() = default;
        ~RenderGraph() { this->Clear(); }

        FORR_CLASS_MOVABLE(RenderGraph)
        FORR_CLASS_NONCOPYABLE(RenderGraph)

        template <typename RenderPassData, RenderPassTraits<RenderPassData> RenderPass>
        FORR_NODISCARD RenderPassHandle<RenderPassData> AddPass(std::string_view name) {
            auto& render_pass = m_RenderPasses.emplace_back();

            std::byte*      mapped_data_raw = m_RenderPassesData.allocate(sizeof(std::decay_t<RenderPassData>), alignof(RenderPassData));
            RenderPassData* mapped_data     = new (mapped_data_raw) RenderPassData();

            render_pass.name             = name;
            render_pass.mapped_data      = mapped_data;
            render_pass.setup_function   = &RenderPass::Setup;
            render_pass.execute_function = [](RenderGraphContext& context, void* ptr) { RenderPass::Execute(context, *reinterpret_cast<RenderPassData*>(ptr)); };
            render_pass.destroy_function = [](void* ptr) { std::destroy_at(reinterpret_cast<RenderPassData*>(ptr)); };

            RenderPassHandle<RenderPassData> render_pass_handle{};
            render_pass_handle.mapped_data       = mapped_data;
            render_pass_handle.render_pass_index = m_RenderPasses.size() - 1;

            return render_pass_handle;
        }

        RenderGraphCompileResult Compile();
        // after 'fe::RenderGraph::Compile()' you get 'fe::RenderGraphCompileResult' - you have to pass it into the renderer
        //  renderer should get you 'fe::RenderGraphBindings' to use it here
        // basically, this function changes 'fe::RenderPass::compiled_barriers->hashed_name' to 'fe::RenderPass::compiled_barriers->texture_index' ( it is a union )
        void SetupResourceBindings(const RenderGraphBindings& bindings);

        render_graph::CommandList Execute(const entt::registry& render_data);

        void Clear();

    private:
        // run Setup() for every added render pass and collect its required resources via fe::RenderGraphBuilder
        void collectRenderPasses(std::vector<CompiledRenderPass>& render_passes_dst, std::unordered_map<fe::StringHash, Resource>& resources_map);

        // run Kahn's algorithm
        void sortRenderPasses(std::vector<CompiledRenderPass>& render_passes_dst);

        // run culling : remove unused render passes
        void removeUnusedRenderPasses(std::vector<CompiledRenderPass>& render_passes_dst, std::unordered_map<fe::StringHash, Resource>& resources_map);

        // translate 'fe::RenderGraph::CompiledRenderPass' to 'fe::RenderPass' and setup 'fe::RenderPass::compiled_barriers'
        void translateRenderPasses(std::vector<CompiledRenderPass>&              render_passes,
                                   std::unordered_map<fe::StringHash, Resource>& resources_map_dst,
                                   std::vector<RenderPass>&                      used_render_passes_dst);

        // setup resource lifetimes
        void calculateResourceLifetimes(std::unordered_map<fe::StringHash, ResourceLifetime>& resource_lifetimes);

        // setup virtual indices
        template <typename AcquireFn, typename ReleaseFn>
        void setupVirtualIndices(AcquireFn&&                                                  acquire_func,
                                 ReleaseFn&&                                                  release_func,
                                 std::unordered_map<fe::StringHash, ResourceLifetime>&        resource_lifetimes,
                                 std::unordered_map<fe::StringHash, render_graph::ImageDesc>& hashed_to_desc_map,
                                 std::unordered_map<fe::StringHash, size_t>&                  hashed_to_virtual_map_dst) {

            for (uint32_t i = 0; i < m_RenderPasses.size(); i++) {
                RenderPass& render_pass = m_RenderPasses[i];

                for (const auto& [hashed_name, lifetime] : resource_lifetimes) {
                    if (lifetime.first_pass_index == i) {
                        const render_graph::ImageDesc& image_desc = hashed_to_desc_map[hashed_name];
                        hashed_to_virtual_map_dst[hashed_name]    = acquire_func(image_desc);
                    }
                }

                for (render_graph::ImageBarrier& image_barrier : render_pass.compiled_barriers) {
                    image_barrier.texture_index = hashed_to_virtual_map_dst[image_barrier.hashed_name];
                }

                for (const auto& [hashed_name, lifetime] : resource_lifetimes) {
                    if (lifetime.last_pass_index == i) {
                        const render_graph::ImageDesc& image_desc            = hashed_to_desc_map[hashed_name];
                        size_t                         virtual_storage_index = hashed_to_virtual_map_dst[hashed_name];
                        release_func(image_desc, virtual_storage_index);
                    }
                }
            }
        }

    private:
        std::vector<RenderPass> m_RenderPasses{};
        fe::Arena               m_RenderPassesData{ 16 * 1024 };

        render_graph::CommandList m_RenderCommands{};
    };

} // namespace fe

template <>
struct std::hash<fe::RenderGraph::ResourceHandle> {
    std::size_t operator()(const fe::RenderGraph::ResourceHandle& handle) const {
        std::size_t h1 = hash<fe::StringHash>()(handle.hashed_name);
        std::size_t h2 = hash<uint32_t>()(handle.version);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

template <>
struct std::hash<fe::render_graph::ImageDesc> {
    std::size_t operator()(const fe::render_graph::ImageDesc& p) const {
        std::size_t seed{};

        // don't use 'fe::render_graph::ImageDesc::hash' here

        fe::render_graph::hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(p.type)));
        fe::render_graph::hash_combine(seed, std::hash<uint8_t>{}(static_cast<uint8_t>(p.format)));
        fe::render_graph::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(p.extent.x)));
        fe::render_graph::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(p.extent.y)));
        fe::render_graph::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(p.extent.z)));
        fe::render_graph::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(p.mip_levels)));
        fe::render_graph::hash_combine(seed, std::hash<uint32_t>{}(static_cast<uint32_t>(p.usage)));

        return seed;
    }
};
