/*===============================================

    Forr Engine

    File : CameraSystem.hpp
    Role : calculates 'ViewProjectionComponent' via 'CameraComponent' and 'TransformComponent'

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include "Forr/ECS/Components.hpp"
#include "Forr/Graphics/IRenderer.hpp"

#include "Forr/ECS/Systems.hpp"

namespace fe {
    class FORR_API CameraSystem {
    public:
        CameraSystem(entt::registry& registry);
        ~CameraSystem();

        void OnUpdate();

    private:
        ReactiveRegistry     m_Registry;
        ReactiveDirtyStorage m_DirtyStorage{};
    };
} // namespace fe
