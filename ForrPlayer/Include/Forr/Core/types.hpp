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
    // this is going to be removed soon
    //struct FORR_NODISCARD MeshID { // like "using MeshID = size_t;"
    //private:
    //    size_t id{};

    //public:
    //    constexpr void                  set(size_t id) noexcept { this->id = id; }
    //    constexpr FORR_NODISCARD size_t get() const noexcept { return id; }

    //    MeshID()  = default;
    //    ~MeshID() = default;

    //    constexpr MeshID(size_t v) noexcept : id(v) {}

    //    constexpr operator size_t() const noexcept { return id; }
    //};

    enum class TextureColorSpace {
        LINEAR,
        SRGB
    };

    enum class TextureInternalFormat {
        RGBA8,
        RGB8,
        RG8,
        R8,
        SRGB8_ALPHA8,
        SRGB8
    };

    enum class TextureDataFormat {
        RGBA,
        RGB,
        RG,
        RED
    };

    enum class TextureMinFilter {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR
    };

    enum class TextureMagFilter {
        NEAREST,
        LINEAR,
    };

    enum class TextureWrap {
        CLAMP_TO_EDGE,
        MIRRORED_REPEAT,
        REPEAT
    };
} // namespace fe
