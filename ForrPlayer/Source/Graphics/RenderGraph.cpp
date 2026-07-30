/*===============================================

    Forr Engine

    File : RenderGraph.cpp
    Role : render graph and interface for render pass

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/RenderGraph.hpp"

fe::RenderGraphCompileResult fe::RenderGraph::Compile() {
    std::vector<CompiledRenderPass> render_passes{};
    render_passes.reserve(m_RenderPasses.size());

    // resource hash --> { resource version, previous state }
    std::unordered_map<fe::StringHash, Resource> resources_map{};

    // run Setup() for every added render pass and collect its required resources via fe::RenderGraphBuilder
    this->collectRenderPasses(render_passes, resources_map);

    // run Kahn's algorithm
    this->sortRenderPasses(render_passes);

    // run culling : remove unused render passes
    this->removeUnusedRenderPasses(render_passes, resources_map);
    // after culling 'resources_map' is broken, so we can't use it anymore
    resources_map.clear();

    // this is needed to translate 'fe::RenderGraph::CompiledRenderPass' to 'fe::RenderPass'
    std::vector<RenderPass> used_render_passes{};
    used_render_passes.reserve(render_passes.size());

    // translate 'fe::RenderGraph::CompiledRenderPass' to 'fe::RenderPass' and setup 'fe::RenderPass::compiled_barriers'
    this->translateRenderPasses(render_passes, resources_map, used_render_passes);

    m_RenderPasses.clear();
    m_RenderPasses = std::move(used_render_passes);

    std::unordered_map<fe::StringHash, ResourceLifetime> resource_lifetimes{};

    // setup resource lifetimes
    this->calculateResourceLifetimes(resource_lifetimes);

    struct PoolImageInfo {
        bool   is_busy{};
        size_t image_storage_index{}; // this is a virtual storage index
    };
    // image desc --> { is busy,  }
    std::unordered_map<render_graph::ImageDesc, std::vector<PoolImageInfo>> image_pool{};

    // this value is used to generate virtual storage indices
    size_t virtual_storage_index_number{};

    // returns image virtual storage index
    auto acquire_image_labmda = [&image_pool, &virtual_storage_index_number](const render_graph::ImageDesc& image_desc) -> size_t {
        auto& pool_vector = image_pool[image_desc];

        for (auto& pool_image_info : pool_vector) {
            if (!pool_image_info.is_busy) {

                pool_image_info.is_busy = true;
                return pool_image_info.image_storage_index;
            }
        }

        PoolImageInfo& pool_image_info      = pool_vector.emplace_back();
        pool_image_info.is_busy             = true;
        pool_image_info.image_storage_index = virtual_storage_index_number++;

        return pool_image_info.image_storage_index;
    };

    auto release_image_lambda = [&image_pool](const render_graph::ImageDesc& image_desc, size_t image_storage_index) {
        auto& pool_vector = image_pool[image_desc];
        for (auto& pool_image_info : pool_vector) {
            if (pool_image_info.image_storage_index == image_storage_index)
                pool_image_info.is_busy = false;
        }
    };

    // hashed name --> image desc
    std::unordered_map<fe::StringHash, render_graph::ImageDesc> hashed_to_desc_map{};
    hashed_to_desc_map.reserve(render_passes.size() * 5);
    for (const auto& render_pass : render_passes) {
        for (const auto& create_request : render_pass.create_requests) {
            hashed_to_desc_map[create_request.hashed_name] = create_request;
        }
    }

    // hashed name --> virtual storage index
    std::unordered_map<fe::StringHash, size_t> hashed_to_virtual_map{};

    // setup virtual indices
    this->setupVirtualIndices(acquire_image_labmda,
                              release_image_lambda,
                              resource_lifetimes,
                              hashed_to_desc_map,
                              hashed_to_virtual_map);

    RenderGraphCompileResult result{};
    result.image_descs.reserve(render_passes.size() * 5);

    for (CompiledRenderPass& render_pass : render_passes) {
        for (render_graph::ImageDesc& image_desc : render_pass.create_requests) {

            image_desc.texture_index = hashed_to_virtual_map[image_desc.hashed_name];
            result.image_descs.emplace_back(image_desc);
        }
    }

    //
    // Pass01 | read : []               , write :  ColorBuffer_v0
    // Pass02 | read :  ColorBuffer_v0  , write :  ColorBuffer_v1
    // Pass03 | read :  ColorBuffer_v1  , write :  OtherBuffer_v0
    // Pass04 | read :  OtherBuffer_v0  , write : OtherBuffer2_v0
    // Pass05 | read : OtherBuffer2_v0  , write :  ColorBuffer_v2
    // Pass06 | read :  ColorBuffer_v2  , write : OtherBuffer3_v0
    // Pass07 | read : OtherBuffer3_v0  , write :  ColorBuffer_v3
    //
    // ColorBuffer --> Backbuffer

    return result;
}

void fe::RenderGraph::SetupResourceBindings(const RenderGraphBindings& bindings) {
    for (RenderPass& render_pass : m_RenderPasses) {
        for (render_graph::ImageBarrier& image_barrier : render_pass.compiled_barriers) {
            auto it = bindings.bindings.find(image_barrier.hashed_name);

            if (it != bindings.bindings.end()) {
                image_barrier.texture_index = it->second;
            }
            else {
                fe::logging::error("Failed to set image barrier's texture index in fe::RenderGraph::SetupResourceBindings. Binding is missing for hash : %llu",
                                   image_barrier.hashed_name);
            }
        }
    }
}

fe::render_graph::CommandList fe::RenderGraph::Execute(const entt::registry& render_data) {
    render_graph::CommandList render_command_list{};
    render_command_list.reserve(m_RenderPasses.size() * 1024);

    for (const RenderPass& render_pass : m_RenderPasses) {

        RenderGraphContext context{ render_data };
        render_pass.execute_function(context, render_pass.mapped_data);

        render_command_list.append_command_list(context.command_list);

        for (const auto& barrier : render_pass.compiled_barriers)
            render_command_list.enqueue(barrier);
    }

    return render_command_list;
}

void fe::RenderGraph::Clear() {
    for (auto& pass : m_RenderPasses) {
        if (pass.destroy_function && pass.mapped_data) {
            pass.destroy_function(pass.mapped_data);
        }
    }
    m_RenderPasses.clear();
    m_RenderPassesData.reset();
    m_RenderCommands.clear();
}

void fe::RenderGraph::collectRenderPasses(std::vector<CompiledRenderPass>&              render_passes_dst,
                                          std::unordered_map<fe::StringHash, Resource>& resources_map) {
    for (auto& render_pass : m_RenderPasses) {
        RenderGraphBuilder builder{};
        render_pass.setup_function(builder);

        auto& this_render_pass = render_passes_dst.emplace_back();

        this_render_pass.reads.reserve(builder.reads.size());
        this_render_pass.writes.reserve(builder.writes.size());

        for (const auto& read_barrier : builder.reads) {
            this_render_pass.reads.emplace_back(CompiledRenderPass::ImageBarrier{ ResourceHandle{ read_barrier.hashed_name, resources_map[read_barrier.hashed_name].version },
                                                                                  resources_map[read_barrier.hashed_name].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                                  read_barrier.new_state });
            resources_map[read_barrier.hashed_name].old_state = read_barrier.new_state;
        }

        for (const auto& write_barrier : builder.writes) {
            resources_map[write_barrier.hashed_name].version++; // increasing version because while writing the resource is changes
            this_render_pass.writes.emplace_back(CompiledRenderPass::ImageBarrier{ ResourceHandle{ write_barrier.hashed_name, resources_map[write_barrier.hashed_name].version },
                                                                                   resources_map[write_barrier.hashed_name].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                                   write_barrier.new_state });
            resources_map[write_barrier.hashed_name].old_state = write_barrier.new_state;
        }

        this_render_pass.create_requests = std::move(builder.create_requests);
        this_render_pass.name            = render_pass.name;
    }
}

void fe::RenderGraph::sortRenderPasses(std::vector<CompiledRenderPass>& render_passes_dst) {
    // a map, there every render pass refers to its dependencies
    std::vector<std::vector<uint32_t>> map(render_passes_dst.size());

    auto add_dependency_lambda = [&](uint32_t dependency_pass, uint32_t current_pass) {
        auto& dependencies = map[dependency_pass];
        if (dependencies.empty() || dependencies.back() != current_pass) {
            dependencies.emplace_back(current_pass);
        }
    };

    // resource handle --> render pass index
    std::unordered_map<ResourceHandle, uint32_t> last_writer{};
    // resource handle --> render passes indices
    std::unordered_map<ResourceHandle, std::vector<uint32_t>> current_readers{};

    for (size_t i = 0; i < render_passes_dst.size(); i++) {
        const auto& this_render_pass = render_passes_dst[i];

        for (const auto& read_barrier : this_render_pass.reads) {
            auto it = last_writer.find(read_barrier.handle);
            if (it != last_writer.end()) {
                add_dependency_lambda(it->second, i);
            }
            current_readers[read_barrier.handle].emplace_back(i);
        }

        for (const auto& write_barrier : this_render_pass.writes) {
            auto readers_it = current_readers.find(write_barrier.handle);

            if (readers_it != current_readers.end()) {
                for (uint32_t reader_pass : readers_it->second) {
                    if (reader_pass != i) {
                        add_dependency_lambda(reader_pass, i);
                    }
                }
                readers_it->second.clear();
            }

            auto writer_it = last_writer.find(write_barrier.handle);
            if (writer_it != last_writer.end()) {
                add_dependency_lambda(writer_it->second, i);
            }

            last_writer[write_barrier.handle] = i;
        }
    }

    std::vector<uint32_t> indegree(render_passes_dst.size(), 0);

    for (uint32_t i = 0; i < render_passes_dst.size(); i++) {
        for (uint32_t j : map[i]) {
            indegree[j]++;
        }
    }

    std::queue<uint32_t> queue{};

    for (uint32_t i = 0; i < render_passes_dst.size(); i++) {
        if (indegree[i] == 0) {
            queue.push(i);
        }
    }

    std::vector<uint32_t> final_list{};
    final_list.reserve(render_passes_dst.size());

    while (!queue.empty()) {
        uint32_t top = queue.front();
        queue.pop();
        final_list.emplace_back(top);

        for (uint32_t j : map[top]) {
            indegree[j]--;

            if (indegree[j] == 0) queue.push(j);
        }
    }

    if (final_list.size() != render_passes_dst.size()) {
        fe::logging::fatal("Failed to compile RenderGraph : failed to run Kahn's algorithm - cycles found");
        return;
    }

    std::vector<CompiledRenderPass> sorted_render_passes{};
    sorted_render_passes.reserve(render_passes_dst.size());

    for (uint32_t i : final_list) {
        sorted_render_passes.emplace_back(std::move(render_passes_dst[i]));
    }

    render_passes_dst = std::move(sorted_render_passes);
}

void fe::RenderGraph::removeUnusedRenderPasses(std::vector<CompiledRenderPass>&              render_passes_dst,
                                               std::unordered_map<fe::StringHash, Resource>& resources_map) {
    std::vector<CompiledRenderPass> used_render_passes{};
    used_render_passes.reserve(render_passes_dst.size());

    std::unordered_set<ResourceHandle> used_resources{};

    constexpr static fe::StringHash final_resource_hash = fe::string_hash("ColorBuffer"); // TODO : rewrite this - use main color image logic instead
    uint32_t                        final_version       = resources_map[final_resource_hash].version;
    used_resources.insert(ResourceHandle{ final_resource_hash, final_version });

    auto add_used_resource_lambda = [&used_resources, &used_render_passes](CompiledRenderPass& CompiledRenderPass) {
        used_resources.reserve(used_resources.size() + CompiledRenderPass.reads.size());
        for (const auto& read_barrier : CompiledRenderPass.reads) {
            used_resources.insert(read_barrier.handle);
        }
        used_render_passes.emplace_back(std::move(CompiledRenderPass));
    };

    for (auto& render_pass : std::views::reverse(render_passes_dst)) {
        bool is_needed = std::ranges::any_of(render_pass.writes, [&used_resources](const CompiledRenderPass::ImageBarrier& write_barrier) {
            return used_resources.contains(write_barrier.handle);
        });

        if (is_needed) {
            add_used_resource_lambda(render_pass);
        }
    }

    std::ranges::reverse(used_render_passes);

    render_passes_dst = std::move(used_render_passes);
}

void fe::RenderGraph::translateRenderPasses(std::vector<CompiledRenderPass>&              render_passes,
                                            std::unordered_map<fe::StringHash, Resource>& resources_map_dst,
                                            std::vector<RenderPass>&                      used_render_passes_dst) {

    for (const CompiledRenderPass& render_pass : render_passes) {
        // find this render pass in 'fe::RenderGraph::m_RenderPasses'
        auto it = std::ranges::find_if(m_RenderPasses, [&render_pass](const RenderPass& this_render_pass) -> bool {
            return render_pass.name == this_render_pass.name;
        });

        if (it != m_RenderPasses.end()) {
            it->compiled_barriers.reserve(render_pass.reads.size() + render_pass.writes.size());

            for (const CompiledRenderPass::ImageBarrier& read_barrier : render_pass.reads) {
                auto& this_barrier = it->compiled_barriers.emplace_back();

                this_barrier.hashed_name = read_barrier.handle.hashed_name;
                this_barrier.old_state   = resources_map_dst[this_barrier.hashed_name].old_state;
                this_barrier.new_state   = read_barrier.new_state;

                resources_map_dst[this_barrier.hashed_name].old_state = read_barrier.new_state;
            }

            for (const CompiledRenderPass::ImageBarrier& write_barrier : render_pass.writes) {
                auto& this_barrier = it->compiled_barriers.emplace_back();

                this_barrier.hashed_name = write_barrier.handle.hashed_name;
                this_barrier.old_state   = resources_map_dst[this_barrier.hashed_name].old_state;
                this_barrier.new_state   = write_barrier.new_state;

                resources_map_dst[this_barrier.hashed_name].old_state = write_barrier.new_state;
            }

            used_render_passes_dst.emplace_back(std::move(*it));
        }
    }
}

void fe::RenderGraph::calculateResourceLifetimes(std::unordered_map<fe::StringHash, ResourceLifetime>& resource_lifetimes) {
    resource_lifetimes.reserve(m_RenderPasses.size() * 5);
    for (size_t i = 0; i < m_RenderPasses.size(); i++) {
        const RenderPass& render_pass = m_RenderPasses[i];

        for (const render_graph::ImageBarrier& image_barrier : render_pass.compiled_barriers) {
            auto& lifetime = resource_lifetimes[image_barrier.hashed_name];

            if (lifetime.first_pass_index == std::numeric_limits<uint32_t>::max()) {
                lifetime.first_pass_index = i;
            }

            lifetime.last_pass_index = i;
        }
    }
}
