/*===============================================

    Forr Engine

    File : ResourceManagementContext.hpp
    Role : resource management context. Contains default variables and current graphics backend

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Core/types.hpp"
#include "Core/pointer.hpp"
#include "Resources.hpp"

namespace fe {
    struct ResourceManagementContext {
        GraphicsBackend graphics_backend{};

        fe::pointer<resource::Material> default_pbr_material_ptr{};

        ResourceManagementContext()  = default;
        ~ResourceManagementContext() = default;
    };
} // namespace fe
