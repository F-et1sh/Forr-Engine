/*===============================================

    Forr Engine

    File : Components.hpp
    Role : ECS ( Entity System ) components.
        All structures here must be POD - this ECS is data-oriented.

        TODO : think about adding that "DOD fe::component::Transform --> OOP fe::TransformComponent" like in Resource Management system

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <variant>

#include "Forr/Core/pointer.hpp"
#include "Forr/ResourceManagement/Resources.hpp"

#ifdef FORRENGINE_EXPORTS
#define ENTT_API_EXPORT
#else
#define ENTT_API_IMPORT
#endif

#include <entt/entt.hpp>

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

    namespace detail {
        template <typename... Ts>
        struct TupleRemoveLast;

        template <typename T, typename... Ts>
        struct TupleRemoveLast<T, Ts...> {
            using type = decltype(std::tuple_cat(std::declval<std::tuple<T>>(),
                                                 std::declval<typename TupleRemoveLast<Ts...>::type>()));
        };

        template <typename Last>
        struct TupleRemoveLast<Last> {
            using type = std::tuple<>;
        };
    } // namespace detail

#define FORR_COMPONENTS_LIST(X) \
    X(Transform)                \
    X(LocalToWorld)             \
    X(Camera)                   \
    X(ViewProjection)           \
    X(Light)                    \
    X(Model)

#define FORR_GENERATE_COMPONENTS_TUPLE(COMPONENT_NAME) COMPONENT_NAME,
    using AllComponents = detail::TupleRemoveLast<FORR_COMPONENTS_LIST(FORR_GENERATE_COMPONENTS_TUPLE) void>::type;
#undef FORR_GENERATE_COMPONENTS_TUPLE

} // namespace fe::component
