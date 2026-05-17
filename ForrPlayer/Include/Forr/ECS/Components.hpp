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

        TransformComponent(const glm::mat4& transform)
            : transform(transform) {}

        TransformComponent()  = default;
        ~TransformComponent() = default;
    };

    struct MeshComponent {
        fe::pointer<resource::Model>    model_ptr{};
        fe::pointer<resource::Material> material_override_ptr{ static_cast<uint64_t>(~0) };

        MeshComponent(fe::pointer<resource::Model> model_ptr, fe::pointer<resource::Material> material_override_ptr)
            : model_ptr(model_ptr), material_override_ptr(material_override_ptr) {}

        MeshComponent(fe::pointer<resource::Model> model_ptr) : model_ptr(model_ptr) {}

        MeshComponent()  = default;
        ~MeshComponent() = default;
    };

    struct LightComponent {
        float     intensity = 1.0f;
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };

        LightComponent()  = default;
        ~LightComponent() = default;
    };
} // namespace fe
