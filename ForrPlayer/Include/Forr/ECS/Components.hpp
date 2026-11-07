/*===============================================

    Forr Engine

    File : Components.hpp
    Role : ECS ( Entity Component System ) components.
        All structures here must be POD - this ECS is data-oriented.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Core/pointer.hpp"
#include "Forr/ResourceManagement/Resources.hpp"
#include "Forr/Graphics/Camera.hpp"

// TODO : think about making this 'fe::component' and removing that '*Component' postfix
namespace fe {
    struct TransformComponent {
        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };

    struct CameraComponent {
        //glm::mat4
    };

    struct LocalToWorldComponent {
        glm::mat4 matrix{};
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
