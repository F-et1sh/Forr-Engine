/*===============================================

    Forr Engine

    File : DefaultRenderPasses.hpp
    Role : render passes that provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "RenderGraph.hpp"

namespace fe {
    struct ForwardPassData {};

    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder) {
            builder.createImage({ .hash       = fe::string_hash("Color"),
                                  .type       = fe::ImageType::IMAGE_TYPE_2D,
                                  .format     = fe::Format::RGBA8_SRGB,
                                  .extent     = glm::ivec3{ 1920, 1080, 1 },
                                  .mip_levels = 1,
                                  .usage      = fe::ImageUsageBits::RENDER_TARGET });

            builder.writeImage(fe::string_hash("Color"), fe::ResourceState::RENDER_TARGET);
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
            
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };
} // namespace fe
