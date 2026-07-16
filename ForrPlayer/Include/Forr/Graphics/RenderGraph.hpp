/*===============================================

    Forr Engine

    File : RenderGraph.hpp
    Role : render graph and an interface for render passes

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <variant>

#include "GPUTypes.hpp"

namespace fe {
    using ImageID  = fe::StringHash;
    using BufferID = fe::StringHash;

    struct CreateImageCommand {
        ImageID        id{};
        ImageType      type{};
        Format         format{};
        glm::ivec3     extent{};
        uint32_t       mip_levels{};
        ImageUsageBits usage{};

        CreateImageCommand()  = default;
        ~CreateImageCommand() = default;
    };

    struct CreateBufferCommand {
        BufferID        id{};
        uint64_t        size{};
        BufferUsageBits usage{};

        CreateBufferCommand()  = default;
        ~CreateBufferCommand() = default;
    };

    struct ImageBarrierCommand {
        ImageID       id{};
        ResourceState old_state{};
        ResourceState new_state{};

        ImageBarrierCommand()  = default;
        ~ImageBarrierCommand() = default;
    };

    struct BufferBarrierCommand {
        BufferID      id{};
        ResourceState old_state{};
        ResourceState new_state{};

        BufferBarrierCommand()  = default;
        ~BufferBarrierCommand() = default;
    };

    struct BeginRenderPassCommand {
        std::vector<ImageID>   color_attachments{};
        std::optional<ImageID> depth_attachment{};
        Rect2D                 render_area{};

        BeginRenderPassCommand()  = default;
        ~BeginRenderPassCommand() = default;
    };

    struct EndRenderPassCommand {};

    struct RenderGraphContext {
        // ...
    };

    struct ExecuteLambdaCommand {
        std::move_only_function<void(RenderGraphContext&)> execute_function{};
    };

    using RenderGraphCommand = std::variant<CreateImageCommand,
                                            CreateBufferCommand,
                                            ImageBarrierCommand,
                                            BufferBarrierCommand,
                                            BeginRenderPassCommand,
                                            EndRenderPassCommand,
                                            ExecuteLambdaCommand>;

    using RenderGraphCommandList = std::vector<RenderGraphCommand>;

    class RenderGraphBuilder {
    private:
        struct ResourceUsageInfo {
            fe::StringHash texture_name{};
            ResourceState  expected_state{};
        };

    public:
        RenderGraphBuilder()  = default;
        ~RenderGraphBuilder() = default;

        FORR_CLASS_MOVABLE(RenderGraphBuilder);
        FORR_CLASS_NONCOPYABLE(RenderGraphBuilder);

        void ReadTexture(fe::StringHash texture_name) {
            m_Reads.emplace_back(ResourceUsageInfo{ texture_name, ResourceState::SHADER_READ });
        }

        void WriteTexture(fe::StringHash texture_name) {
            m_Writes.emplace_back(ResourceUsageInfo{ texture_name, ResourceState::RENDER_TARGET });
        }

    private:
        std::vector<ResourceUsageInfo> m_Reads;
        std::vector<ResourceUsageInfo> m_Writes;

        friend class RenderGraph;
    };

    class RenderGraph {
    public:
        RenderGraph()  = default;
        ~RenderGraph() = default;

        template <typename FuncSetup, typename FuncExecute, typename Data>
        void AddPass(const fe::fixed_string<32>& name, FuncSetup&& setup_lambda, FuncExecute execute_lambda, Data& data) {
            
        }

    private:
        RenderGraphCommandList m_CommandList{};
    };

    struct ForwardPassData {
        // ...

        ForwardPassData()  = default;
        ~ForwardPassData() = default;
    };

    struct ForwardPass {
        fe::fixed_string<32> name{};

        void Setup(RenderGraphBuilder& builder) const {
            builder.WriteTexture(fe::string_hash("Color"));
        }

        void Execute(RenderGraphContext& context, ForwardPassData& pass_data) const {
            // ...

            /*
            auto& model_matrices = context.gpu_storage.GetShaderBuffer(fe::string_hash("model_matrices"));

            */
        }

        ForwardPass()  = default;
        ~ForwardPass() = default;
    };

} // namespace fe
