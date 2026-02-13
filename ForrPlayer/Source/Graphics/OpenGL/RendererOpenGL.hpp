/*===============================================

    Forr Engine

    File : RendererOpenGL.hpp
    Role : OpenGL Renderer implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Graphics/IRenderer.hpp"

namespace fe {
    class RendererOpenGL : public IRenderer {
    public:
        RendererOpenGL(const RendererDesc& desc);
        ~RendererOpenGL();
    };
} // namespace fe
