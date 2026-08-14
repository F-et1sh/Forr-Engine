/*===============================================

    Forr Engine

    File : RenderSystem.hpp
    Role : renderer system

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ECS/Components.hpp"
#include "Graphics/IRenderer.hpp"

#include "entt/fwd.hpp"

namespace fe {
    struct RenderMeshEntry {
        uint32_t index_offset{};
        uint32_t index_count{};

        // TODO : change this to fe::pointer<>
        GPUHandle<resource::Model::Mesh>    mesh_handle{};
        fe::pointer<fe::resource::Material> material_ptr{};

        uint64_t sort_key{};

        RenderMeshEntry()  = default;
        ~RenderMeshEntry() = default;
    };

    class RenderSystem {
    public:
        RenderSystem(ResourceManager& resource_manager, entt::registry& registry, IRenderer& renderer, RenderPacket& render_packet);
        ~RenderSystem();

        void Update();

    private:
        //void handleMeshComponents();
        //void handleLightComponents();
        
        //void addEntry(const MeshComponent& mesh_component);
        //void addToDrawList(fe::pointer<resource::Model> model_ptr, const glm::mat4& transform);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl{};
    };
} // namespace fe
