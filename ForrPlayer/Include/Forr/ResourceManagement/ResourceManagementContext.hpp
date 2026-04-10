/*===============================================

    Forr Engine

    File : ResourceManagementContext.hpp
    Role : resource management context

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

        fe::pointer<resource::Shader>   default_gltf_vertex_shader_ptr{};
        fe::pointer<resource::Shader>   default_gltf_fragment_shader_ptr{};
        fe::pointer<resource::Material> default_gltf_material_ptr{};

        ResourceManagementContext()  = default;
        ~ResourceManagementContext() = default;
    };
} // namespace fe
