/*===============================================

    Forr Engine

    File : IRenderer.cpp
    Role : Renderer interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Graphics/IRenderer.hpp"

#include "OpenGL/RendererOpenGL.hpp"
#include "Vulkan/RendererVulkan.hpp"

std::unique_ptr<fe::IRenderer> fe::IRenderer::Create(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index, ResourceManager& resource_manager) {
    std::unique_ptr<fe::IRenderer> result{};

    RendererDesc this_desc = desc;
    fe::IRenderer::createDefaultMaterials(this_desc, resource_manager);

    switch (desc.graphics_backend) {
        case GraphicsBackend::OpenGL:
            result = std::make_unique<RendererOpenGL>(this_desc, platform_system, primary_window_index, resource_manager);
            break;
        case GraphicsBackend::Vulkan:
            result = std::make_unique<RendererVulkan>(this_desc, platform_system, primary_window_index, resource_manager);
            break;
        default:
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one", desc.graphics_backend);

            result = std::make_unique<RendererOpenGL>(this_desc, platform_system, primary_window_index, resource_manager);
            break;
    }

    return std::move(result);
}

void fe::IRenderer::createDefaultMaterials(RendererDesc& desc, ResourceManager& resource_manager) {
    resource::Material material{};
    material.vertex_shader_ptr   = resource_manager.ImportResource<resource::Shader>(PATH.getShadersPath() / "default3.vert");
    material.fragment_shader_ptr = resource_manager.ImportResource<resource::Shader>(PATH.getShadersPath() / "default.frag");
    //material.buffer = ...
    if (!desc.default_gltf_material_ptr) desc.default_gltf_material_ptr = resource_manager.CreateResource(std::move(material));
}
