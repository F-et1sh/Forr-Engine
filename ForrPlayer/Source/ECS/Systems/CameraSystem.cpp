/*===============================================

    Forr Engine

    File : CameraSystem.cpp
    Role : calculates 'ViewProjection' via 'Camera' and 'LocalToWorld'

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/ECS/Systems/CameraSystem.hpp"

#include <entt/entt.hpp>

fe::CameraSystem::CameraSystem(entt::registry& registry) : m_Registry(registry) {
    m_DirtyStorage.bind(registry);

    fe::bind_to_construct_of<component::Camera, component::LocalToWorld>(m_DirtyStorage);
    fe::bind_to_update_of<component::Camera, component::LocalToWorld>(m_DirtyStorage);
}

void fe::CameraSystem::OnUpdate() {
    for (auto [entity, camera_component, local_to_world_component] : m_DirtyStorage.view<component::Camera, component::LocalToWorld>().each()) {
        component::ViewProjection view_projection_component{};

        std::visit([&view_projection_component](auto&& camera_settigs) {
            using T = std::decay_t<decltype(camera_settigs)>;
            if constexpr (std::is_same_v<T, component::Camera::PerspectiveSettings>) {
                view_projection_component.projection = glm::perspective(camera_settigs.fov, camera_settigs.aspect_ratio, camera_settigs.near_plane, camera_settigs.far_plane);
            }
            else if constexpr (std::is_same_v<T, component::Camera::OrthographicSettings>) {
                view_projection_component.projection = glm::ortho(camera_settigs.left, camera_settigs.right, camera_settigs.bottom, camera_settigs.top);
            }
        },
                   camera_component.camera_settings);

        view_projection_component.view            = glm::inverse(local_to_world_component.matrix);
        view_projection_component.view_projection = view_projection_component.projection * view_projection_component.view;

        m_Registry.emplace_or_replace<component::ViewProjection>(entity, view_projection_component);
    }

    m_DirtyStorage.clear();
}
