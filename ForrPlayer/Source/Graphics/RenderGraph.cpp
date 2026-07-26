/*===============================================

    Forr Engine

    File : RenderGraph.cpp
    Role : render graph and interface for render pass

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/RenderGraph.hpp"

fe::render_graph::CreateCommandList fe::RenderGraph::Compile() {
    std::vector<Node> render_passes{};
    render_passes.reserve(m_RenderPasses.size());

    // resource hash --> { resource version, previous state }
    std::unordered_map<fe::StringHash, Resource> resources_map{};

    // run Setup() for every added render pass and collect its required resources via fe::RenderGraphBuilder
    this->collectRenderPasses(render_passes, resources_map);

    // run Kahn's algorithm
    this->sortRenderSasses(render_passes);

    // run culling : remove unused render passes
    this->removeUnusedRenderPasses(render_passes, resources_map);
    // after culling 'resource_map' is broken, so we can't use it anymore
    resources_map.clear();

    render_graph::CreateCommandList create_command_list{};
    // forecast that every render pass wants to create 2 resources
    create_command_list.reserve(render_passes.size() * 2);

    std::vector<RenderPass> used_render_passes{};
    used_render_passes.reserve(render_passes.size());

    // create all resources
    for (const Node& node : render_passes) {
        for (const render_graph::ImageDesc& create_request : node.create_requests) {
            create_command_list.emplace_back(create_request);
        }

        // find this render pass in 'fe::RenderGraph::m_RenderPasses'
        auto it = std::ranges::find_if(m_RenderPasses, [&node](const RenderPass& render_pass) -> bool {
            return render_pass.name == node.name;
        });

        if (it != m_RenderPasses.end()) {
            it->compiled_barriers.reserve(node.reads.size() + node.writes.size());

            for (const Node::ImageBarrier& read_barrier : node.reads) {
                auto& this_barrier = it->compiled_barriers.emplace_back();

                this_barrier.hash      = read_barrier.handle.hash;
                this_barrier.old_state = resources_map[this_barrier.hash].old_state;
                this_barrier.new_state = read_barrier.new_state;

                resources_map[this_barrier.hash].old_state = read_barrier.new_state;
            }

            for (const Node::ImageBarrier& write_barrier : node.writes) {
                auto& this_barrier = it->compiled_barriers.emplace_back();

                this_barrier.hash      = write_barrier.handle.hash;
                this_barrier.old_state = resources_map[this_barrier.hash].old_state;
                this_barrier.new_state = write_barrier.new_state;

                resources_map[this_barrier.hash].old_state = write_barrier.new_state;
            }

            used_render_passes.emplace_back(std::move(it));
        }
    }

    m_RenderPasses.clear();
    m_RenderPasses = std::move(used_render_passes);

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

    return create_command_list;
}

fe::render_graph::RenderCommandList fe::RenderGraph::Execute() {
    render_graph::RenderCommandList render_command_list{};
    render_command_list.reserve(m_RenderPasses.size() * 10);

    for (auto& render_pass : m_RenderPasses) {
        RenderGraphContext context{};
        render_pass.execute_function(context, render_pass.mapped_data);

        render_command_list.append_range(context.render_command_list);
        render_command_list.append_range(render_pass.compiled_barriers);
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

void fe::RenderGraph::collectRenderPasses(std::vector<Node>&                            render_passes_dst,
                                          std::unordered_map<fe::StringHash, Resource>& resources_map) {
    for (auto& render_pass : m_RenderPasses) {
        RenderGraphBuilder builder{};
        render_pass.setup_function(builder);

        auto& this_render_pass = render_passes_dst.emplace_back();

        this_render_pass.reads.reserve(builder.reads.size());
        this_render_pass.writes.reserve(builder.writes.size());

        for (const auto& read_barrier : builder.reads) {
            this_render_pass.reads.emplace_back(Node::ImageBarrier{ ResourceHandle{ read_barrier.hash, resources_map[read_barrier.hash].version },
                                                                    resources_map[read_barrier.hash].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                    read_barrier.new_state });
            resources_map[read_barrier.hash].old_state = read_barrier.new_state;
        }

        for (const auto& write_barrier : builder.writes) {
            resources_map[write_barrier.hash].version++; // increasing version because while writing the resource is changes
            this_render_pass.writes.emplace_back(Node::ImageBarrier{ ResourceHandle{ write_barrier.hash, resources_map[write_barrier.hash].version },
                                                                     resources_map[write_barrier.hash].old_state, // 'read_barrier.old_state' won't work here because it set by default
                                                                     write_barrier.new_state });
            resources_map[write_barrier.hash].old_state = write_barrier.new_state;
        }

        this_render_pass.create_requests = std::move(builder.create_requests);
        this_render_pass.name            = render_pass.name;
    }
}

void fe::RenderGraph::sortRenderSasses(std::vector<Node>& render_passes_dst) {
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

    std::vector<Node> sorted_render_passes{};
    sorted_render_passes.reserve(render_passes_dst.size());

    for (uint32_t i : final_list) {
        sorted_render_passes.emplace_back(std::move(render_passes_dst[i]));
    }

    render_passes_dst = std::move(sorted_render_passes);
}

void fe::RenderGraph::removeUnusedRenderPasses(std::vector<Node>&                            render_passes_dst,
                                               std::unordered_map<fe::StringHash, Resource>& resources_map) {
    std::vector<Node> used_render_passes{};
    used_render_passes.reserve(render_passes_dst.size());

    std::unordered_set<ResourceHandle> used_resources{};

    constexpr static fe::StringHash final_resource_hash = fe::string_hash("ColorBuffer"); // TODO : rewrite this - use main color image logic instead
    uint32_t                        final_version       = resources_map[final_resource_hash].version;
    used_resources.insert(ResourceHandle{ final_resource_hash, final_version });

    auto add_used_resource_lambda = [&used_resources, &used_render_passes](Node& node) {
        used_resources.reserve(used_resources.size() + node.reads.size());
        for (const auto& read_barrier : node.reads) {
            used_resources.insert(read_barrier.handle);
        }
        used_render_passes.emplace_back(std::move(node));
    };

    for (auto& render_pass : std::views::reverse(render_passes_dst)) {
        bool is_needed = std::ranges::any_of(render_pass.writes, [&used_resources](const Node::ImageBarrier& write_barrier) {
            return used_resources.contains(write_barrier.handle);
        });

        if (is_needed) {
            add_used_resource_lambda(render_pass);
        }
    }

    std::ranges::reverse(used_render_passes);

    render_passes_dst = std::move(used_render_passes);
}
