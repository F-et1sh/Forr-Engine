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
    struct FORR_NODISCARD MeshID { // like "using MeshID = size_t;"
    private:
        size_t id{};

    public:
        constexpr void                  set(size_t id) noexcept { this->id = id; }
        constexpr FORR_NODISCARD size_t get() const noexcept { return id; }

        MeshID()  = default;
        ~MeshID() = default;

        constexpr MeshID(size_t v) noexcept : id(v) {}

        constexpr operator size_t() const noexcept { return id; }
    };

    enum class RenderMode {
        POINTS,
        LINES,
        LINE_LOOP,
        LINE_STRIP,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
    };

    enum class RenderIndexType {
        UNSIGNED_BYTE,
        UNSIGNED_SHORT,
        UNSIGNED_INT,
    };
} // namespace fe
