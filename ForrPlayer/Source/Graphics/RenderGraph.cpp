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
    this->gatherGraph();
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

void fe::RenderGraph::gatherGraph() {
    for (auto& render_pass : m_RenderPasses) {
        fe::RenderGraphBuilder builder{};

        render_pass.setup_function(builder);

        for (auto& build_command : builder.build_commands) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, RenderGraphBuilder::ImageDesc>) {
                    m_RequestCommands.emplace_back(RenderGraph::ImageDesc{ arg.type, arg.format, arg.extent, arg.mip_levels, arg.usage });
                }
                else if constexpr (std::is_same_v<T, RenderGraphBuilder::ImageBarrier>) {
                    
                }
            },
                       build_command);
        }
    }
}
