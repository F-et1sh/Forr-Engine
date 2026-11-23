/*===============================================

    Forr Engine

    File : ResourceManagementContext.hpp
    Role : resource management context. Contains default variables and current graphics backend

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include "Forr/Core/pointer.hpp"
#include "Resources.hpp"

namespace fe {
    struct ResourceManagementContext {
        GraphicsBackend graphics_backend{};

        fe::pointer<resource::Material> default_pbr_material_ptr{};

        ResourceManagementContext()  = default;
        ~ResourceManagementContext() = default;
    };
} // namespace fe
