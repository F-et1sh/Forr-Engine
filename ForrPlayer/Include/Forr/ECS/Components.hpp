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
    struct TransformComponent {
        glm::mat4 transform{}; // temp

        TransformComponent()  = default;
        ~TransformComponent() = default;
    };

    struct MeshComponent {
        fe::pointer<resource::Model> model_ptr{};
        uint32_t                     mesh_id = ~0; // ~0 means that renderer has to draw all meshes

        MeshComponent()  = default;
        ~MeshComponent() = default;
    };
} // namespace fe
