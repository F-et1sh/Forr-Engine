/*===============================================

    Forr Engine

    File : RenderGraph.hpp
    Role : render graph and an interface for render passes

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    struct RenderGraphContext {
        // ...

        RenderGraphContext()  = default;
        ~RenderGraphContext() = default;

        FORR_CLASS_MOVABLE(RenderGraphContext);
        FORR_CLASS_NONCOPYABLE(RenderGraphContext);
    };
    
    struct RenderGraphBuilder {
        // ...

        RenderGraphBuilder()  = default;
        ~RenderGraphBuilder() = default;

        FORR_CLASS_MOVABLE(RenderGraphBuilder);
        FORR_CLASS_NONCOPYABLE(RenderGraphBuilder);
    };

    struct ForwardPassData {
        // ...

        ForwardPassData()  = default;
        ~ForwardPassData() = default;
    };

    struct ForwardPass {
        fe::fixed_string<32> name{};

        void Initialize(RenderGraphContext& context, ForwardPassData& pass_data) const {
            // ...
        }

        void Execute(RenderGraphContext& context, ForwardPassData& pass_data) const {
            // ...
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

    class RenderGraph {
    public:
        RenderGraph()  = default;
        ~RenderGraph() = default;

        void AddPass() {
        }

    private:
        fe::Arena m_Arena{ 32 * 16 };
    };

} // namespace fe
