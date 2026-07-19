/*===============================================

    Forr Engine

    File : RenderGraph.cpp
    Role : render graph and interface for render pass

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/RenderGraph.hpp"

struct ResourceHandle {
    fe::StringHash hash{};
    uint32_t       version{};

    ResourceHandle() = default;
    ResourceHandle(fe::StringHash hash, uint32_t version) : hash(hash), version(version) {}

    bool operator==(const ResourceHandle& other) const noexcept = default;
};

template <>
struct std::hash<ResourceHandle> {
    std::size_t operator()(const ResourceHandle& handle) const {
        std::size_t h1 = hash<fe::StringHash>()(handle.hash);
        std::size_t h2 = hash<uint32_t>()(handle.version);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

struct Node { // render pass
    struct ImageBarrier {
        ResourceHandle    handle{};
        fe::ResourceState to_state{};

        ImageBarrier() = default;
        ImageBarrier(const ResourceHandle& handle, fe::ResourceState to_state) : handle(handle), to_state(to_state) {}
    };

    std::vector<Node::ImageBarrier>                reads{};
    std::vector<Node::ImageBarrier>                writes{};
    std::vector<fe::RenderGraphBuilder::ImageDesc> create_requests{};

    Node() = default;
    Node(std::vector<Node::ImageBarrier> reads, std::vector<Node::ImageBarrier> writes, std::vector<fe::RenderGraphBuilder::ImageDesc> create_requests)
        : reads(std::move(reads)), writes(std::move(writes)), create_requests(std::move(create_requests)) {}

    FORR_CLASS_MOVABLE(Node)
    FORR_CLASS_NONCOPYABLE(Node)
};

void fe::RenderGraph::Compile() {
    std::vector<Node> render_passes{};
    render_passes.reserve(m_RenderPasses.size());

    // resource hash --> resource version
    std::unordered_map<fe::StringHash, uint32_t> resource_versions{};

    for (auto& render_pass : m_RenderPasses) {
        RenderGraphBuilder builder{};
        render_pass.setup_function(builder);

        auto& this_render_pass = render_passes.emplace_back();

        this_render_pass.reads.reserve(builder.reads.size());
        this_render_pass.writes.reserve(builder.writes.size());

        for (const auto& read_barrier : builder.reads) {
            this_render_pass.reads.emplace_back(Node::ImageBarrier{ ResourceHandle{ read_barrier.hash, resource_versions[read_barrier.hash] }, read_barrier.to_state });
        }
        for (const auto& write_barrier : builder.writes) {
            resource_versions[write_barrier.hash]++; // increasing version
            this_render_pass.writes.emplace_back(Node::ImageBarrier{ ResourceHandle{ write_barrier.hash, resource_versions[write_barrier.hash] }, write_barrier.to_state });
        }

        this_render_pass.create_requests = std::move(builder.create_requests);
    }

    // a map, there every render pass refers to its dependencies
    std::vector<std::vector<uint32_t>> map(render_passes.size());

    auto add_dependency = [&](uint32_t dependency_pass, uint32_t current_pass) {
        auto& dependencies = map[dependency_pass];
        if (dependencies.empty() || dependencies.back() != current_pass) {
            dependencies.emplace_back(current_pass);
        }
    };

    // resource handle --> render pass index
    std::unordered_map<ResourceHandle, uint32_t> last_writer{};
    // resource handle --> render passes indices
    std::unordered_map<ResourceHandle, std::vector<uint32_t>> current_readers{};

    for (size_t i = 0; i < render_passes.size(); i++) {
        const auto& this_render_pass = render_passes[i];

        for (const auto& read_barrier : this_render_pass.reads) {
            auto it = last_writer.find(read_barrier.handle);
            if (it != last_writer.end()) {
                add_dependency(it->second, i);
            }
            current_readers[read_barrier.handle].emplace_back(i);
        }

        for (const auto& write_barrier : this_render_pass.writes) {
            auto readers_it = current_readers.find(write_barrier.handle);

            if (readers_it != current_readers.end()) {
                for (uint32_t reader_pass : readers_it->second) {
                    if (reader_pass != i) {
                        add_dependency(reader_pass, i);
                    }
                }
                readers_it->second.clear();
            }

            auto writer_it = last_writer.find(write_barrier.handle);
            if (writer_it != last_writer.end()) {
                add_dependency(writer_it->second, i);
            }

            last_writer[write_barrier.handle] = i;
        }
    }

    std::vector<uint32_t> indegree(render_passes.size(), 0);

    for (uint32_t i = 0; i < render_passes.size(); i++) {
        for (uint32_t j : map[i]) {
            indegree[j]++;
        }
    }

    std::queue<uint32_t> queue{};

    for (uint32_t i = 0; i < render_passes.size(); i++) {
        if (indegree[i] == 0) {
            queue.push(i);
        }
    }

    std::vector<uint32_t> final_list{};
    final_list.reserve(render_passes.size());

    while (!queue.empty()) {
        uint32_t top = queue.front();
        queue.pop();
        final_list.emplace_back(top);

        for (uint32_t j : map[top]) {
            indegree[j]--;

            if (indegree[j] == 0) queue.push(j);
        }
    }

    if (final_list.size() != render_passes.size()) {
        fe::logging::fatal("Failed to compile RenderGraph : cycles found");
        return;
    }

    std::vector<Node> sorted_render_passes{};
    sorted_render_passes.reserve(render_passes.size());

    for (uint32_t i : final_list) {
        sorted_render_passes.emplace_back(std::move(render_passes[i]));
    }

    // TODO : provde culling
    // TODO : make command requests

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
}

void fe::RenderGraph::Clear() {
    for (auto& pass : m_RenderPasses) {
        if (pass.destroy_function && pass.mapped_data) {
            pass.destroy_function(pass.mapped_data);
        }
    }
    m_RenderPasses.clear();
    m_RenderPassesData.reset();
    m_RequestCommands.clear();
}
