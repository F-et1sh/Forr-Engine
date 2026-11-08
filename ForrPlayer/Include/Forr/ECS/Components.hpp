/*===============================================

    Forr Engine

    File : Components.hpp
    Role : ECS ( Entity Component System ) components.
        All structures here must be POD - this ECS is data-oriented.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <variant>

#include "Forr/Core/pointer.hpp"
#include "Forr/ResourceManagement/Resources.hpp"
#include "Forr/Graphics/Camera.hpp"

// TODO : think about making this 'fe::component' and removing that '*Component' postfix
namespace fe {
    struct TransformComponent {
        glm::vec3 position{};
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };

    struct LocalToWorldComponent {
        glm::mat4 matrix{};
    };

    struct CameraComponent {
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

    struct ViewProjectionComponent {
        glm::mat4 projection{};
        glm::mat4 view{};
        glm::mat4 view_projection{};
    };

    struct LightComponent {
        glm::mat4 transform{}; // temp

        LightComponent(const glm::mat4& transform)
            : transform(transform) {}

        LightComponent() = default;
    };

    struct ModelComponent {
        fe::pointer<resource::Model> model_ptr{};

        ModelComponent(fe::pointer<resource::Model> model_ptr) : model_ptr(model_ptr) {}
        ModelComponent() = default;
    };
} // namespace fe
