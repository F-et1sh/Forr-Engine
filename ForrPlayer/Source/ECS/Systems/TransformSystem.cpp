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
}

void fe::TransformSystem::OnUpdate() {
    auto view = m_Registry.get().view<TransformComponent>();

    for (auto entity : view) {
        TransformComponent&    transform_component      = m_Registry.get().get<TransformComponent>(entity);
        LocalToWorldComponent& local_to_world_component = m_Registry.get().get_or_emplace<LocalToWorldComponent>(entity);

        auto& matrix = local_to_world_component.matrix;
        matrix       = glm::mat4(1.0f);
        matrix       = glm::translate(matrix, transform_component.position);
        matrix       = glm::rotate(matrix, transform_component.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        matrix       = glm::rotate(matrix, transform_component.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        matrix       = glm::rotate(matrix, transform_component.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        matrix       = glm::scale(matrix, transform_component.scale);
    }
}
