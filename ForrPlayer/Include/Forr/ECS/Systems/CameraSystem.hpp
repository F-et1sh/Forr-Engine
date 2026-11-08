/*===============================================

    Forr Engine

    File : CameraSystem.hpp
    Role : calculates 'ViewProjectionComponent' via 'CameraComponent' and 'TransformComponent'

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/ECS/Components.hpp"
#include "Forr/Graphics/IRenderer.hpp"

#include <entt/fwd.hpp>

namespace fe {
    class FORR_API CameraSystem {
    public:
        CameraSystem(entt::registry& registry);
        ~CameraSystem() = default;

        void OnUpdate();

    private:
        std::reference_wrapper<entt::registry>    m_Registry;
        entt::reactive_mixin<entt::storage<void>> m_DirtyStorage{};
    };
} // namespace fe
