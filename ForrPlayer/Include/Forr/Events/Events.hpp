/*===============================================

    Forr Engine

    File : Events.hpp
    Role : default events provided by the engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <cinttypes>

#include "Forr/Core/attributes.hpp"
#include "Forr/Input/InputTypes.hpp"

namespace fe::events {
    struct KeyButton {
        input::KeyCode  key{};
        input::Action   action{};
        input::KeyMods  mods{};
        input::ScanCode scan_code{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & input::KeyMods::SHIFT; }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & input::KeyMods::CONTROL; }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & input::KeyMods::ALT; }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & input::KeyMods::SUPER; }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & input::KeyMods::CAPS_LOCK; }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & input::KeyMods::NUM_LOCK; }
    };

    struct MouseButton {
        input::MouseButton button{};
        input::Action      action{};
        input::KeyMods     mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & input::KeyMods::SHIFT; }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & input::KeyMods::CONTROL; }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & input::KeyMods::ALT; }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & input::KeyMods::SUPER; }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & input::KeyMods::CAPS_LOCK; }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & input::KeyMods::NUM_LOCK; }
    };

    struct MouseMoved {
        double x{};
        double y{};
    };

    struct MouseScrolled {
        double x_offset{};
        double y_offset{};
    };

    struct UnicodeCharTyped {
        input::UnicodeChar code_point{};
    };

    // TODO : add more event structures

} // namespace fe::events
