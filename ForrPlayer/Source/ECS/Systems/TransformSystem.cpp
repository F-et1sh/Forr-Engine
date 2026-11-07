/*===============================================

    Forr Engine

    File : TransformSystem.cpp
    Role : calculates 'LocalToWorldComponent' via 'TransformComponent' and 'CameraComponent'

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
        TransformComponent&    transform_component      = m_Registry.get().get<TransformComponent>(entity);
        LocalToWorldComponent& local_to_world_component = m_Registry.get().get_or_emplace<LocalToWorldComponent>(entity);

        glm::mat4 rotation              = glm::toMat4(transform_component.rotation);
        local_to_world_component.matrix = glm::translate(glm::mat4(1.0f), transform_component.position) *
                                          rotation *
                                          glm::scale(glm::mat4(1.0f), transform_component.scale);
    }
    m_DirtyStorage.clear();
}
