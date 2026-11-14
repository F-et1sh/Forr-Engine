/*===============================================

    Forr Engine

    File : Components.hpp
    Role : ECS ( Entity System ) components.
        All structures here must be POD - this ECS is data-oriented.

        TODO : think about adding that "DOD fe::component::Transform --> OOP fe::TransformComponent" like in Resource Management system

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <variant>

#include "Forr/Core/pointer.hpp"
#include "Forr/ResourceManagement/Resources.hpp"

#include <entt/fwd.hpp>

namespace fe {
    template <typename... Components>
    static constexpr void bind_to_update_of(entt::reactive_mixin<entt::storage<void>>& dirty_storage) noexcept {
        (dirty_storage.on_update<Components>(), ...);
    }

    template <typename... Components>
    static constexpr void bind_to_construct_of(entt::reactive_mixin<entt::storage<void>>& dirty_storage) noexcept {
        (dirty_storage.on_construct<Components>(), ...);
    }

    template <typename... Components>
    static constexpr void bind_to_destroy_of(entt::reactive_mixin<entt::storage<void>>& dirty_storage) noexcept {
        (dirty_storage.on_destroy<Components>(), ...);
    }
} // namespace fe

namespace fe::component {
    struct Transform {
        glm::vec3 position{};
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };

    struct LocalToWorld {
        glm::mat4 matrix{};
    };

    struct Camera {
        struct PerspectiveSettings {
            float fov{};
            float aspect_ratio{};

            float near_plane{};
            float far_plane{};
        };

        struct OrthographicSettings {
            float left{};
            float right{};
            float bottom{};
            float top{};

            float near_plane{};
            float far_plane{};
        };

        std::variant<PerspectiveSettings,
                     OrthographicSettings>
             camera_settings{};
        bool is_primary{};
    };

    struct ViewProjection {
        glm::mat4 projection{};
        glm::mat4 view{};
        glm::mat4 view_projection{};
    };

    struct Light {
        glm::mat4 transform{}; // temp

        Light(const glm::mat4& transform)
            : transform(transform) {}

        Light() = default;
    };

    struct Model {
        fe::pointer<resource::Model> model_ptr{};

        Model(fe::pointer<resource::Model> model_ptr) : model_ptr(model_ptr) {}
        Model() = default;
    };
} // namespace fe::component
