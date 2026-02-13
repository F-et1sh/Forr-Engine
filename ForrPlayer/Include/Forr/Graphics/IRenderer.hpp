/*===============================================

    Forr Engine

    File : IRenderer.hpp
    Role : Renderer interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Platform/IPlatformSystem.hpp"

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

        static std::unique_ptr<IRenderer> Create(const RendererDesc& desc, IPlatformSystem& platform_system, size_t primary_window_index);

        virtual void ClearScreen(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f) = 0;
        virtual void SwapBuffers()                                                                            = 0;
    };
} // namespace fe
