/*===============================================

    Forr Engine

    File : Components.hpp
    Role : ECS ( Entity Component System ) components.
        All structures here must be POD - this ECS is data-oriented.

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/pointer.hpp"
#include "ResourceManagement/Resources.hpp"

namespace fe {
    struct FORR_API TransformComponent {
        glm::mat4 transform{}; // temp

        TransformComponent(const glm::mat4& transform)
            : transform(transform) {}

        TransformComponent()  = default;
    };

    struct FORR_API LightComponent {
        bool      is_static{};
        float     intensity = 1.0f;
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        glm::vec3 direction{};

        LightComponent()  = default;
    };
} // namespace fe
