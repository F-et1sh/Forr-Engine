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
        std::size_t index = ~0;

        GPUHandle()  = default;
        ~GPUHandle() = default;

        explicit GPUHandle(size_t index) : index(index) {}

        bool operator==(GPUHandle<T> other) const noexcept { return index == other.index; }
        bool operator!=(GPUHandle<T> other) const noexcept { return index != other.index; }

        operator bool() const noexcept { return index != ~0; }

        bool operator<(GPUHandle<T> other) const noexcept { return index < other.index; }
        bool operator>(GPUHandle<T> other) const noexcept { return index > other.index; }
    };
} // namespace fe
