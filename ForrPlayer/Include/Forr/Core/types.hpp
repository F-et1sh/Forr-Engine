/*===============================================

    Forr Engine

    File : types.hpp
    Role : different types

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <cstddef>
#include "Core/attributes.hpp"

namespace fe {
    enum class FORR_API GraphicsBackend {
        OpenGL,
        Vulkan
    };

    enum class FORR_API PlatformBackend {
        GLFW
    };
} // namespace fe
