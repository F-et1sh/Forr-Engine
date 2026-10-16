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

namespace fe {
    struct TransformComponent {
        glm::mat4 transform{}; // temp

        TransformComponent(const glm::mat4& transform)
            : transform(transform) {}

        TransformComponent() = default;
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
