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
    struct ShadowPassData {};
    struct ShadowPass {
        static void Setup(RenderGraphBuilder& builder) {
            builder.createImage(render_graph::ImageDesc{ fe::string_hash("ShadowMap1"),
                                                         render_graph::ImageType::IMAGE_TYPE_2D,
                                                         render_graph::Format::RGBA8_SRGB,
                                                         glm::ivec3{ 3840, 2160, 1 },
                                                         1,
                                                         render_graph::ImageUsageBits::RENDER_TARGET });
            builder.writeImage(fe::string_hash("ShadowMap"), ResourceState::RENDER_TARGET);
        }

        static void Execute(RenderGraphContext& context, ShadowPassData& pass_data) {
        }

        ShadowPass()  = default;
        ~ShadowPass() = default;
    };

    struct ForwardPassData {};
    struct ForwardPass {
        static void Setup(RenderGraphBuilder& builder) {
            builder.createImage(render_graph::ImageDesc{ fe::string_hash("ColorBuffer"),
                                                         render_graph::ImageType::IMAGE_TYPE_2D,
                                                         render_graph::Format::RGBA8_SRGB,
                                                         glm::ivec3{ 1920, 1080, 1 },
                                                         1,
                                                         render_graph::ImageUsageBits::RENDER_TARGET });
            builder.readImage(fe::string_hash("ShadowMap"), ResourceState::SHADER_READ_ONLY);
            builder.writeImage(fe::string_hash("ColorBuffer"), ResourceState::RENDER_TARGET);
        }

        static void Execute(RenderGraphContext& context, ForwardPassData& pass_data) {
            auto view = context.render_registry.view<MeshComponent>();
            // view...
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

} // namespace fe
