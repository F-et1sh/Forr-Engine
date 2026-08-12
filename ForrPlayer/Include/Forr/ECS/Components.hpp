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
        ~TransformComponent() = default;
    };

    // TODO : remove
    struct FORR_API MeshComponent {
        fe::pointer<resource::Model>    model_ptr{};
        fe::pointer<resource::Material> material_override_ptr{ static_cast<uint64_t>(~0) };

        MeshComponent(fe::pointer<resource::Model> model_ptr, fe::pointer<resource::Material> material_override_ptr)
            : model_ptr(model_ptr), material_override_ptr(material_override_ptr) {}

        MeshComponent(fe::pointer<resource::Model> model_ptr) : model_ptr(model_ptr) {}

        MeshComponent()  = default;
        ~MeshComponent() = default;
    };

    // TODO : remove 'fe::MeshComponent' and remove '2' from name of this
    struct FORR_API MeshComponent2 {
        fe::pointer<resource::Model>     model_ptr{};
        std::span<resource::Model::Mesh> models_to_draw{};
    };

    struct FORR_API LightComponent {
        bool      is_static{};
        float     intensity = 1.0f;
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        glm::vec3 direction{};

        LightComponent()  = default;
        ~LightComponent() = default;
    };
} // namespace fe
