/*===============================================

    Forr Engine

    File : RenderSystem.hpp
    Role : renderer system. Without EnTT for now

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ECS/Components.hpp"
#include "Graphics/IRenderer.hpp"

namespace fe {
    struct RenderMeshEntry {
        uint32_t index_offset{};
        uint32_t index_count{};

        GPUHandle<resource::Model::Mesh> mesh_handle{};
        GPUHandle<resource::Material>    material_handle{};

        uint64_t sort_key{};

        RenderMeshEntry()  = default;
        ~RenderMeshEntry() = default;
    };

    class RenderSystem {
    public:
        RenderSystem(ResourceManager& resource_manager, entt::registry& registry, IRenderer& renderer)
            : m_ResourceManager(resource_manager), m_Registry(registry), m_Renderer(renderer) {}
        ~RenderSystem() = default;

        void Update();

    private:
        void addEntry(fe::pointer<resource::Model> model_ptr);
        void addToDrawList(fe::pointer<resource::Model> model_ptr, const glm::mat4& transform);

    private:
        ResourceManager&                       m_ResourceManager;
        std::reference_wrapper<entt::registry> m_Registry; // testing std::reference_wrapper<>
        std::reference_wrapper<IRenderer>      m_Renderer; // testing std::reference_wrapper<>

        std::vector<DrawCommand>     m_DrawCommands{};
        std::vector<RenderMeshEntry> m_RenderMeshEntries{};

        std::unordered_map<fe::pointer<resource::Model>, std::vector<RenderMeshEntry>> m_Table{};
    };
} // namespace fe
