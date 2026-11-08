/*===============================================

    Forr Engine

    File : CameraSystem.cpp
    Role : calculates 'ViewProjectionComponent' via 'CameraComponent' and 'LocalToWorldComponent'

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/ECS/Systems/CameraSystem.hpp"

#include <entt/entt.hpp>

fe::CameraSystem::CameraSystem(entt::registry& registry) : m_Registry(registry) {
    m_DirtyStorage.bind(registry);
    m_DirtyStorage.on_construct<CameraComponent>();
    m_DirtyStorage.on_update<CameraComponent>();

    m_DirtyStorage.on_construct<LocalToWorldComponent>();
    m_DirtyStorage.on_update<LocalToWorldComponent>();
}

void fe::CameraSystem::OnUpdate() {
    auto dirty_view = m_DirtyStorage.view<CameraComponent, LocalToWorldComponent>();

    dirty_view.each([&](auto entity, auto& camera_component, auto& local_to_world_component) {
        TransformComponent&      transform_component       = m_Registry.get().get<TransformComponent>(entity);
        ViewProjectionComponent& view_projection_component = m_Registry.get().get_or_emplace<ViewProjectionComponent>(entity);

        view_projection_component.view            = glm::lookAt(transform_component.position,
                                                                glm::vec3(-1.0f, -1.0f, 0.0f),
                                                                glm::vec3(0.0f, 1.0f, 0.0f));
        view_projection_component.projection      = glm::perspective(glm::radians(45.0f),
                                                                     1920.0f / 1080.0f,
                                                                     0.1f,
                                                                     1000.0f);
        view_projection_component.view_projection = view_projection_component.projection * view_projection_component.view;
    });

    //m_DirtyStorage.view<CameraComponent>().each([&](auto entity, auto& camera_component) {
    //    TransformComponent&      transform_component       = m_Registry.get().get<TransformComponent>(entity);
    //    LocalToWorldComponent&   local_to_world_component  = m_Registry.get().get<LocalToWorldComponent>(entity);
    //    ViewProjectionComponent& view_projection_component = m_Registry.get().get_or_emplace<ViewProjectionComponent>(entity);

    //    //std::visit([&](auto&& settings) {
    //    //    using T = std::decay_t<decltype(settings)>;
    //    //    if constexpr (std::is_same_v<T, CameraComponent::PerspectiveSettings>) {
    //    //        view_projection_component.projection = glm::perspective(settings.fov, settings.aspect_ratio, settings.near_plane, settings.far_plane);
    //    //    }
    //    //    else if constexpr (std::is_same_v<T, CameraComponent::OrthographicSettings>) {
    //    //        view_projection_component.projection = glm::ortho(settings.left, settings.right, settings.bottom, settings.top, settings.near_plane, settings.far_plane);
    //    //    }
    //    //},
    //    //           camera_component.camera_settings);
    //    //
    //    //view_projection_component.view            = glm::lookAt(glm::vec3(8.0f, 3.0f, 5.0f),
    //    //                                                        glm::vec3(-1.0f, -1.0f, 0.0f),
    //    //                                                        glm::vec3(0.0f, 1.0f, 0.0f));
    //    //view_projection_component.view_projection = view_projection_component.projection * view_projection_component.view;

    //    view_projection_component.view            = glm::lookAt(transform_component.position,
    //                                                            glm::vec3(-1.0f, -1.0f, 0.0f),
    //                                                            glm::vec3(0.0f, 1.0f, 0.0f));
    //    view_projection_component.projection      = glm::perspective(glm::radians(45.0f),
    //                                                                 1920.0f / 1080.0f,
    //                                                                 0.1f,
    //                                                                 1000.0f);
    //    view_projection_component.view_projection = view_projection_component.projection * view_projection_component.view;
    //});

    m_DirtyStorage.clear();
}
