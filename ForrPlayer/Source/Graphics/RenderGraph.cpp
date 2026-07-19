/*===============================================

    Forr Engine

    File : RenderGraph.cpp
    Role : render graph and interface for render pass

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/RenderGraph.hpp"

void fe::RenderGraph::Compile() {
    struct Node { // render pass
        std::vector<RenderGraphBuilder::ImageBarrier> reads{};
        std::vector<RenderGraphBuilder::ImageBarrier> writes{};
        std::vector<RenderGraphBuilder::ImageDesc>    create_requests{};

        FORR_CLASS_MOVABLE(Node)
        FORR_CLASS_NONCOPYABLE(Node)
    };

    std::vector<Node> render_passes{};
    render_passes.reserve(m_RenderPasses.size());

    for (auto& render_pass : m_RenderPasses) {
        RenderGraphBuilder builder{};
        render_pass.setup_function(builder);

        auto& this_render_pass = render_passes.emplace_back();

        this_render_pass.reads           = std::move(builder.reads);
        this_render_pass.writes          = std::move(builder.writes);
        this_render_pass.create_requests = std::move(builder.create_requests);
    }

    // a map, there every render pass refers to its dependencies
    std::vector<std::vector<uint32_t>> map(render_passes.size());

    // resource hash --> render pass index
    std::unordered_map<fe::StringHash, uint32_t> last_writer{};
    // resource hash --> render passes indices
    std::unordered_map<fe::StringHash, std::vector<uint32_t>> current_readers{};

    for (size_t i = 0; i < render_passes.size(); i++) {
        const auto& this_render_pass = render_passes[i];

        for (const auto& read_barrier : this_render_pass.reads) {
            auto it = last_writer.find(read_barrier.hash);
            if (it != last_writer.end()) {

                uint32_t dependency_pass = it->second;
                if (map[dependency_pass].empty() || map[dependency_pass].back() != i) {
                    map[dependency_pass].emplace_back(i);
                }
            }

            current_readers[read_barrier.hash].emplace_back(i);
        }

        for (const auto& write_barrier : this_render_pass.writes) {
            auto readers_it = current_readers.find(write_barrier.hash);

            if (readers_it != current_readers.end()) {
                for (uint32_t reader_pass : readers_it->second) {
                    if (reader_pass != i) {
                        map[reader_pass].emplace_back(i);
                    }
                }
                readers_it->second.clear();
            }

            auto writer_it = last_writer.find(write_barrier.hash);
            if (writer_it != last_writer.end() && writer_it->second != i) {
                map[writer_it->second].emplace_back(i);
            }

            last_writer[write_barrier.hash] = i;
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

    //
    // Pass01 | read : []           , write ColorBuffer
    // Pass02 | read : ColorBuffer  , write ColorBuffer
    // Pass03 | read : ColorBuffer  , write OtherBuffer
    // Pass04 | read : OtherBuffer  , write OtherBuffer2
    // Pass05 | read : OtherBuffer2 , write ColorBuffer
    // Pass06 | read : ColorBuffer  , write OtherBuffer3
    // Pass07 | read : OtherBuffer3 , write ColorBuffer
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
}
