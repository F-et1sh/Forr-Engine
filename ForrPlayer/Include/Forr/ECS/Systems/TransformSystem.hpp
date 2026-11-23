/*===============================================

    Forr Engine

    File : TransformSystem.hpp
    Role : calculates 'LocalToWorldComponent' via 'TransformComponent'

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include "Forr/ECS/Components.hpp"
#include "Forr/Graphics/IRenderer.hpp"

#include "Forr/ECS/Systems.hpp"

namespace fe {
    class FORR_API TransformSystem {
    public:
        TransformSystem(entt::registry& registry);
        ~TransformSystem();

        void OnUpdate();

    private:
        ReactiveRegistry     m_Registry;
        ReactiveDirtyStorage m_DirtyStorage{};
    };
} // namespace fe
