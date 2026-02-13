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

std::unique_ptr<fe::IRenderer> fe::IRenderer::Create(const RendererDesc& desc) {
    std::unique_ptr<fe::IRenderer> result{};

    switch (desc.backend) {
        case GraphicsBackend::OpenGL:
            result = std::make_unique<RendererOpenGL>(desc);
            break;
        default:
            fe::logging::warning("The selected renderer backend %i was not found. Using the default one", desc.backend);

            result = std::make_unique<RendererOpenGL>(desc);
            break;
    }

    return std::move(result);
}
