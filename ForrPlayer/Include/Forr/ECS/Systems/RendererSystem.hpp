/*===============================================

    Forr Engine

    File : RendererSystem.hpp
    Role : renderer system. Without EnTT for now

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "ECS/Components.hpp"
#include "Graphics/IRenderer.hpp"

namespace fe {
    class RendererSystem {
    public:
        RendererSystem(entt::registry& registry, IRenderer& renderer, ResourceManager& resource_manager)
            : m_Registry(registry), m_Renderer(renderer), m_ResourceManager(resource_manager) {}
        ~RendererSystem() = default;

        void Update();

    private:
        std::reference_wrapper<entt::registry> m_Registry; // testing std::reference_wrapper<>
        std::reference_wrapper<IRenderer>      m_Renderer; // testing std::reference_wrapper<>
        ResourceManager&                       m_ResourceManager;
        std::vector<DrawCommand>               m_DrawCommands{};
    };
} // namespace fe
