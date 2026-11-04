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
        uint8_t         mods{};
        input::ScanCode scan_code{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::NUM_LOCK); }
    };

    struct MouseButton {
        input::MouseButton button{};
        input::Action      action{};
        uint8_t            mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::NUM_LOCK); }
    };

    struct MouseMoved {
        double x{};
        double y{};
    };

    struct MouseScrolled {
        float offset_x{};
        float offset_y{};
    };

    // TODO : add more event structures

} // namespace fe::events
