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
#include "Forr/Core/input.hpp"

namespace fe::events {
    struct KeyButton {
        fe::input::KeyCode key{};
        fe::input::Action  action{};
        uint8_t            mods{};
        //uint16_t           scan_code{}; // TODO : provide scan codes logic

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::NUM_LOCK); }
    };

    struct MouseButton {
        fe::input::MouseButton button{};
        fe::input::Action      action{};
        uint8_t                mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(fe::input::KeyMods::NUM_LOCK); }
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
