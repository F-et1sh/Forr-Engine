/*===============================================

    Forr Engine

    File : TransformSystem.cpp
    Role : calculates 'LocalToWorldComponent' via 'TransformComponent'

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/ECS/Systems/TransformSystem.hpp"

#include <entt/entt.hpp>

fe::TransformSystem::TransformSystem(entt::registry& registry) : m_Registry(registry) {
    m_DirtyStorage.bind(registry);

    fe::bind_related_component<component::LocalToWorld, component::Transform>(m_DirtyStorage);
}

fe::TransformSystem::~TransformSystem() {
    fe::unbind_related_component<component::LocalToWorld, component::Transform>(m_DirtyStorage);
}

void fe::TransformSystem::OnUpdate() {
    for (auto entity : m_DirtyStorage) {
        const component::Transform& transform_component = m_Registry.get<component::Transform>(entity);

        component::LocalToWorld local_to_world_component{};

        glm::mat4 rotation              = glm::toMat4(transform_component.rotation);
        local_to_world_component.matrix = glm::translate(glm::mat4(1.0f), transform_component.position) *
                                          rotation *
                                          glm::scale(glm::mat4(1.0f), transform_component.scale);

        m_Registry.emplace_or_replace<component::LocalToWorld>(entity, local_to_world_component);
    }
    m_DirtyStorage.clear();
}
