/*===============================================

    Forr Engine

    File : RenderGraph.hpp
    Role : render graph, interface for render passes and other helpers

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "ResourceManagement/ResourceManager.hpp"
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

        RenderGraphContext& BindShaderProgram(const render_graph::BindShaderProgram& bind_shader_program) {
            command_list.enqueue(bind_shader_program);
            return *this;
        }

        RenderGraphContext& BindShaderProgram(const fe::pointer<resource::ShaderProgram> shader_program_ptr) {
            command_list.enqueue(render_graph::BindShaderProgram{ shader_program_ptr });
            return *this;
        }

        RenderGraphContext& BindMaterial(const render_graph::BindMaterial& bind_material) {
            command_list.enqueue(bind_material);
            return *this;
        }

        RenderGraphContext& BindMaterial(const fe::pointer<resource::Material> material_ptr) {
            command_list.enqueue(render_graph::BindMaterial{ material_ptr });
            return *this;
        }

        RenderGraphContext& DrawIndexed(const render_graph::DrawIndexed& draw_indexed) {
            command_list.enqueue(draw_indexed);
            return *this;
        }

        RenderGraphContext(const entt::registry& render_registry) : render_registry(render_registry) {}
        ~RenderGraphContext() = default;

        FORR_CLASS_MOVABLE(RenderGraphContext)
        FORR_CLASS_NONCOPYABLE(RenderGraphContext)
    };

    // a proxy to gather setup commands from render pass
    struct FORR_API RenderGraphBuilder {
        ResourceManager& resource_manager;

        std::vector<render_graph::ImageBarrier> image_reads{};
        std::vector<render_graph::ImageBarrier> image_writes{};

        std::vector<render_graph::BufferBarrier> buffer_reads{};
        std::vector<render_graph::BufferBarrier> buffer_writes{};

        std::vector<render_graph::ImageDesc>  image_create_requests{};
        std::vector<render_graph::BufferDesc> buffer_create_requests{};

        std::vector<std::string> warnings{};
        std::vector<std::string> errors{};
        std::vector<std::string> fatals{};

        // this render pass will still be in render graph without changes
        RenderGraphBuilder& assertWarning(const std::string& message) {
            warnings.emplace_back(message);
            return *this;
        }

        // this render pass will still be in render graph without changes
        RenderGraphBuilder& assertError(const std::string& message) {
            errors.emplace_back(message);
            return *this;
        }

        // this render pass will be removed from render graph after compiling
        RenderGraphBuilder& assertFatal(const std::string& message) {
            fatals.emplace_back(message);
            return *this;
        }

        RenderGraphBuilder& createImage(const render_graph::ImageDesc& desc) {
            image_create_requests.emplace_back(desc);
            return *this;
        }

        RenderGraphBuilder& createBuffer(const render_graph::BufferDesc& desc) {
            buffer_create_requests.emplace_back(desc);
            return *this;
        }

        RenderGraphBuilder& readImage(fe::StringHash hash, ResourceState to_state) {
            switch (to_state) {
                case ResourceState::RENDER_TARGET:
                case ResourceState::DEPTH_WRITE:
                case ResourceState::UNORDERED_ACCESS:
                case ResourceState::COPY_DST:
                case ResourceState::VERTEX_BUFFER:
                case ResourceState::INDEX_BUFFER:
                case ResourceState::CONSTANT_BUFFER:
                case ResourceState::INDIRECT_ARGUMENT:
                    fe::logging::error("RenderGraphBuilder::readImage() : wrong state %i", to_state);
                    return *this;
                    break;
            }
            image_reads.emplace_back(render_graph::ImageBarrier{ hash, ResourceState::UNDEFINED, to_state });
            return *this;
        }

        RenderGraphBuilder& writeImage(fe::StringHash hash, ResourceState to_state) {
            switch (to_state) {
                case ResourceState::DEPTH_READ:
                case ResourceState::SHADER_READ_ONLY:
                case ResourceState::COPY_SRC:
                case ResourceState::VERTEX_BUFFER:
                case ResourceState::INDEX_BUFFER:
                case ResourceState::CONSTANT_BUFFER:
                case ResourceState::INDIRECT_ARGUMENT:
                    fe::logging::error("RenderGraphBuilder::writeImage() : wrong state %i", to_state);
                    return *this;
                    break;
            }
            image_writes.emplace_back(render_graph::ImageBarrier{ hash, ResourceState::UNDEFINED, to_state });
            return *this;
        }

        RenderGraphBuilder(ResourceManager& resource_manager) : resource_manager(resource_manager) {}
        ~RenderGraphBuilder() = default;

        FORR_CLASS_MOVABLE(RenderGraphBuilder)
        FORR_CLASS_NONCOPYABLE(RenderGraphBuilder)
    };

    // this structure is needed for the 'fe::RenderGraph::Compile()'
    //  it is used by the renderer to create all resources
    struct FORR_API RenderGraphCompileResult {
        std::vector<render_graph::ImageDesc>  image_descs{};
        std::vector<render_graph::BufferDesc> buffer_descs{};

        RenderGraphCompileResult()  = default;
        ~RenderGraphCompileResult() = default;

        FORR_CLASS_MOVABLE(RenderGraphCompileResult)
        FORR_CLASS_NONCOPYABLE(RenderGraphCompileResult)
    };

    // this structure is needed for the 'fe::RenderGraph::SetupResourceBindings()'
    struct FORR_API RenderGraphBindings {
        std::unordered_map<size_t, size_t> image_bindings{};
        std::unordered_map<size_t, size_t> buffer_bindings{};

        RenderGraphBindings()  = default;
        ~RenderGraphBindings() = default;

        FORR_CLASS_MOVABLE(RenderGraphBindings)
        FORR_CLASS_NONCOPYABLE(RenderGraphBindings)
    };

    template <typename T, typename DataT>
    concept RenderPassTraits = requires(RenderGraphBuilder& builder,
                                        RenderGraphContext& context,
                                        DataT&              render_pass_data) {
        { T::Setup(builder, render_pass_data) } -> std::same_as<void>;
        { T::Execute(context, render_pass_data) } -> std::same_as<void>;
    };

    struct RenderPass {
        using SetupFunction   = void (*)(RenderGraphBuilder& builder, void*);
        using ExecuteFunction = void (*)(RenderGraphContext& context, void*);
        using DestroyFunction = void (*)(void*);

        fe::fixed_string<32> name{};
        void*                mapped_data{};

        SetupFunction   setup_function{};
        ExecuteFunction execute_function{};
        DestroyFunction destroy_function{};

        render_graph::BeginRenderPass compiled_begin_command{};
        render_graph::EndRenderPass   compiled_end_command{};

        std::vector<render_graph::ImageBarrier>  compiled_image_barriers{};
        std::vector<render_graph::BufferBarrier> compiled_buffer_barriers{};

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
            struct ResourceBarrier {
                ResourceHandle handle{};
                ResourceState  old_state{};
                ResourceState  new_state{};

                ResourceBarrier() = default;
                ResourceBarrier(const ResourceHandle& handle, ResourceState old_state, ResourceState new_state)
                    : handle(handle), old_state(old_state), new_state(new_state) {}
            };

            using ResourceBarrier = ResourceBarrier;
            using ResourceBarrier = ResourceBarrier;

            fe::fixed_string<32> name{};

            std::vector<ResourceBarrier> image_reads{};
            std::vector<ResourceBarrier> image_writes{};

            std::vector<ResourceBarrier> buffer_reads{};
            std::vector<ResourceBarrier> buffer_writes{};

            std::vector<render_graph::ImageDesc>  image_create_requests{};
            std::vector<render_graph::BufferDesc> buffer_create_requests{};

            CompiledRenderPass() = default;
            CompiledRenderPass(std::string_view                      name,
                               std::vector<ResourceBarrier>          image_reads,
                               std::vector<ResourceBarrier>          image_writes,
                               std::vector<ResourceBarrier>          buffer_reads,
                               std::vector<ResourceBarrier>          buffer_writes,
                               std::vector<render_graph::ImageDesc>  image_create_requests,
                               std::vector<render_graph::BufferDesc> buffer_create_requests)
                : name(name),
                  image_reads(std::move(image_reads)),
                  image_writes(std::move(image_writes)),
                  buffer_reads(std::move(buffer_reads)),
                  buffer_writes(std::move(buffer_writes)),
                  image_create_requests(std::move(image_create_requests)),
                  buffer_create_requests(std::move(buffer_create_requests)) {}

            FORR_CLASS_MOVABLE(CompiledRenderPass)
            FORR_CLASS_NONCOPYABLE(CompiledRenderPass)
        };

    public:
        RenderGraph(ResourceManager& resource_manager) : m_ResourceManager(resource_manager) {}
        ~RenderGraph() { this->Clear(); }

        FORR_CLASS_MOVABLE(RenderGraph)
        FORR_CLASS_NONCOPYABLE(RenderGraph)

        template <typename RenderPassData, RenderPassTraits<RenderPassData> RenderPass>
        FORR_NODISCARD RenderPassHandle<RenderPassData> AddPass(const fe::fixed_string<32>& name) {
            auto& render_pass = m_RenderPasses.emplace_back();

            std::byte*      mapped_data_raw = m_RenderPassesData.allocate(sizeof(std::decay_t<RenderPassData>), alignof(RenderPassData));
            RenderPassData* mapped_data     = new (mapped_data_raw) RenderPassData();

            render_pass.name             = name;
            render_pass.mapped_data      = mapped_data;
            render_pass.setup_function   = [](RenderGraphBuilder& builder, void* render_pass_data) { RenderPass::Setup(builder, *reinterpret_cast<RenderPassData*>(render_pass_data)); };
            render_pass.execute_function = [](RenderGraphContext& context, void* render_pass_data) { RenderPass::Execute(context, *reinterpret_cast<RenderPassData*>(render_pass_data)); };
            render_pass.destroy_function = [](void* render_pass_data) { std::destroy_at(reinterpret_cast<RenderPassData*>(render_pass_data)); };

            RenderPassHandle<RenderPassData> render_pass_handle{};
            render_pass_handle.mapped_data       = mapped_data;
            render_pass_handle.render_pass_index = m_RenderPasses.size() - 1;

            return render_pass_handle;
        }

        RenderGraphCompileResult Compile();
        // after 'fe::RenderGraph::Compile()' you get 'fe::RenderGraphCompileResult' - you have to pass it into the renderer
        //  renderer should get you 'fe::RenderGraphBindings' to use it here
        // basically, this function changes 'fe::RenderPass::compiled_image_barriers->hashed_name' to 'fe::RenderPass::compiled_image_barriers->texture_index'
        //  and the same for 'fe::RenderPass::compiled_buffer_barriers'
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

        // translate 'fe::RenderGraph::CompiledRenderPass' to 'fe::RenderPass' and setup 'fe::RenderPass::compiled_image_barriers' and 'fe::RenderPass::compiled_buffer_barriers'
        void translateRenderPasses(std::vector<CompiledRenderPass>&              render_passes,
                                   std::unordered_map<fe::StringHash, Resource>& resources_map_dst,
                                   std::vector<RenderPass>&                      used_render_passes_dst);

        // setup resource lifetimes
        void calculateResourceLifetimes(std::unordered_map<fe::StringHash, ResourceLifetime>& resource_lifetimes);

        // setup virtual indices
        template <typename AcquireFn, typename ReleaseFn>
        void setupVirtualIndices(AcquireFn&&                                                        acquire_func,
                                 ReleaseFn&&                                                        release_func,
                                 std::unordered_map<fe::StringHash, ResourceLifetime>&              resource_lifetimes,
                                 std::unordered_map<fe::StringHash, render_graph::CreationCommand>& hashed_to_desc_map,
                                 std::unordered_map<fe::StringHash, size_t>&                        hashed_to_virtual_map_dst) {

            for (uint32_t i = 0; i < m_RenderPasses.size(); i++) {
                RenderPass& render_pass = m_RenderPasses[i];

                for (const auto& [hashed_name, lifetime] : resource_lifetimes) {
                    if (lifetime.first_pass_index == i) {
                        const render_graph::CreationCommand& desc = hashed_to_desc_map[hashed_name];
                        hashed_to_virtual_map_dst[hashed_name]    = acquire_func(desc);
                    }
                }

                for (render_graph::ImageBarrier& image_barrier : render_pass.compiled_image_barriers) {
                    image_barrier.handle.storage_index = hashed_to_virtual_map_dst[image_barrier.handle.hashed_name];
                }

                for (render_graph::BufferBarrier& buffer_barrier : render_pass.compiled_buffer_barriers) {
                    buffer_barrier.handle.storage_index = hashed_to_virtual_map_dst[buffer_barrier.handle.hashed_name];
                }

                for (const auto& [hashed_name, lifetime] : resource_lifetimes) {
                    if (lifetime.last_pass_index == i) {
                        const render_graph::CreationCommand& desc                  = hashed_to_desc_map[hashed_name];
                        size_t                               virtual_storage_index = hashed_to_virtual_map_dst[hashed_name];
                        release_func(desc, virtual_storage_index);
                    }
                }
            }
        }

    private:
        ResourceManager& m_ResourceManager;

        std::vector<RenderPass> m_RenderPasses{};
        fe::Arena               m_RenderPassesData{ 16 * 1024 };
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
