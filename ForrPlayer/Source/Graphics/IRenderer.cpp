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

std::unique_ptr<fe::IRenderer> fe::IRenderer::Create(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index) {
    std::unique_ptr<fe::IRenderer> result{};

    switch (desc.graphics_backend) {
        case GraphicsBackend::OpenGL:
            result = std::make_unique<RendererOpenGL>(desc, platform_system, primary_window_index);
            break;
        case GraphicsBackend::Vulkan:
            result = std::make_unique<RendererVulkan>(desc, platform_system, primary_window_index);
            break;
        default:
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one", desc.graphics_backend);

            result = std::make_unique<RendererOpenGL>(desc, platform_system, primary_window_index);
            break;
    }

    return std::move(result);
}
