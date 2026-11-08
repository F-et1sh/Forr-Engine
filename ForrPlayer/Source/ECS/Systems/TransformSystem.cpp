/*===============================================

    Forr Engine

    File : TransformSystem.cpp
    Role : calculates 'LocalToWorldComponent' via 'TransformComponent'

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/ECS/Systems/TransformSystem.hpp"

#include <entt/entt.hpp>

fe::TransformSystem::TransformSystem(entt::registry& registry) : m_Registry(registry) {
    m_DirtyStorage.bind(registry);
    m_DirtyStorage.on_construct<TransformComponent>();
    m_DirtyStorage.on_update<TransformComponent>();
}

void fe::TransformSystem::OnUpdate() {
    for (auto entity : m_DirtyStorage) {
        TransformComponent&   transform_component = m_Registry.get().get<TransformComponent>(entity);

        LocalToWorldComponent local_to_world_component{};

        glm::mat4 rotation              = glm::toMat4(transform_component.rotation);
        local_to_world_component.matrix = glm::translate(glm::mat4(1.0f), transform_component.position) *
                                          rotation *
                                          glm::scale(glm::mat4(1.0f), transform_component.scale);

        m_Registry.get().emplace_or_replace<LocalToWorldComponent>(entity, local_to_world_component);
    }
    m_DirtyStorage.clear();
}
