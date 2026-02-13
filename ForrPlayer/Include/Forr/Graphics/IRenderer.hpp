/*===============================================

    Forr Engine

    File : IRenderer.hpp
    Role : Renderer interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    enum class FORR_API GraphicsBackend {
        OpenGL
    };

    struct FORR_API RendererDesc {
        GraphicsBackend backend;
    };

    class FORR_API IRenderer {
    public:
        virtual ~IRenderer() = default;

        static std::unique_ptr<IRenderer> Create(const RendererDesc& desc);
    };
} // namespace fe
