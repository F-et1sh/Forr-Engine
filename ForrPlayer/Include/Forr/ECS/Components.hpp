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
#include <utility>
#include <variant>

#include "Forr/Core/pointer.hpp"
#include "Forr/ResourceManagement/Resources.hpp"

#include <entt/entt.hpp>

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

    class ReactiveRegistry {
    public:
        ReactiveRegistry(entt::registry& registry) : m_registry(registry) {}

        template <typename T, typename Func>
        decltype(auto) patch(entt::entity entity, Func&& func) {
            return std::as_const(m_registry.get().patch<T>(entity, std::forward<Func>(func)));
        }

        template <typename T, typename... Args>
        decltype(auto) emplace_or_replace(entt::entity entity, Args&&... args) {
            return std::as_const(m_registry.get().emplace_or_replace<T>(entity, std::forward<Args>(args)...));
        }

        template <typename T, typename... Args>
        decltype(auto) replace(entt::entity entity, Args&&... args) {
            return std::as_const(m_registry.get().replace<T>(entity, std::forward<Args>(args)...));
        }

        template <typename T>
        FORR_NODISCARD decltype(auto) get(entt::entity entity) const {
            return std::as_const(m_registry.get().get<T>(entity));
        }

        FORR_NODISCARD const entt::registry& registry() const { return m_registry; }
        const entt::registry& change_registry(entt::registry& registry_to) { return m_registry = registry_to; }

    private:
        std::reference_wrapper<entt::registry> m_registry;
    };

    using ReactiveDirtyStorage = entt::reactive_mixin<entt::storage<void>>;
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
