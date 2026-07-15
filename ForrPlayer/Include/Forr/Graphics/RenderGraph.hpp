/*===============================================

    Forr Engine

    File : RenderGraph.hpp
    Role : render graph and an interface for render passes

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    // RenderPass :
    //  Owns    - name, data
    //  Needs   - context
    //  Has     - Initialize() and Execute()

    // RenderGraph :
    //  Owns    - RenderPasses
    //  Needs   - context
    //  Has     - ...

    class FORR_API RenderGraph {
    public:
        RenderGraph() = default;
        ~RenderGraph() = default;

        private:
            //std::vector<>
    };

} // namespace fe
