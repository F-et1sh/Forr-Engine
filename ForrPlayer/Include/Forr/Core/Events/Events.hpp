/*===============================================

    Forr Engine

    File : Events.hpp
    Role : default events provided by the engine
        Based on GLFW ( zlib License. Copyright (c) 2002-2006 Marcus Geelnard, Copyright (c) 2006-2019 Camilla Lowy )
                                                                                                                ^ changed the letter, because clang-format doesn't work with non-ASCII

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <cinttypes>

#include "Forr/Core/attributes.hpp"
#include "Forr/Core/inputs.hpp"

namespace fe::events {
    struct KeyEvent {
        fe::inputs::KeyCode     key{};
        fe::inputs::InputAction action{};
        uint8_t                 mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_NUM_LOCK); }
    };

    struct MouseButtonEvent {
        fe::inputs::MouseButton button{};
        fe::inputs::InputAction action{};
        uint8_t                 mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(fe::inputs::KeyMods::MOD_NUM_LOCK); }
    };

    struct MouseMovedEvent {
        float x{};
        float y{};
    };

    struct MouseScrolledEvent {
        float offset_x{};
        float offset_y{};
    };

    // TODO : add all needed structures and create translator from GLFW to Unified somewhere in Source/Platform/GLFW

} // namespace fe::events
