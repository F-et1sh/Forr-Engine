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

    // GPU handle to be put in CPU resource
    // this is needed for GPU resource managers
    template <typename T>
    struct GPUHandle {
        size_t index{};

        GPUHandle()  = default;
        ~GPUHandle() = default;

        explicit GPUHandle(size_t index) : index(index) {}
    };
} // namespace fe
