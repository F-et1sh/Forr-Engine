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
    for (auto& render_pass : m_RenderPasses) {
        RenderGraphBuilder builder{};
        render_pass.setup_function(builder);
        //builder.m_Writes;
    }
}

void fe::RenderGraph::Execute(fe::RenderPacket& render_packet) {

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
