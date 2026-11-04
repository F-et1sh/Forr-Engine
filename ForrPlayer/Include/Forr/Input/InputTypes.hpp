/*===============================================

    Forr Engine

    File : InputTypes.hpp
    Role : enum classes of key codes, mouse buttons etc

    Based on GLFW ( zlib License. Copyright (c) 2002-2006 Marcus Geelnard, Copyright (c) 2006-2019 Camilla Lowy )
                                                                                                            ^ changed the letter, because clang-format doesn't work with non-ASCII

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#pragma once
#include <cinttypes>
#include <array>

namespace fe::input {
    enum class KeyCode : uint8_t {
        UNKNOWN = 0,

        /* Printable keys */
        SPACE,
        APOSTROPHE, /* ' */
        COMMA,      /* , */
        MINUS,      /* - */
        PERIOD,     /* . */
        SLASH,      /* / */
        NUMBER_0,
        NUMBER_1,
        NUMBER_2,
        NUMBER_3,
        NUMBER_4,
        NUMBER_5,
        NUMBER_6,
        NUMBER_7,
        NUMBER_8,
        NUMBER_9,
        SEMICOLON, /* ; */
        EQUAL,     /* = */
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LEFT_BRACKET,  /* [ */
        BACKSLASH,     /* \ */
        RIGHT_BRACKET, /* ] */
        GRAVE_ACCENT,  /* ` */
        WORLD_1,       /* non-US #1 */
        WORLD_2,       /* non-US #2 */

        /* Function keys */
        ESCAPE,
        ENTER,
        TAB,
        BACKSPACE,
        INSERT,
        DELETE,
        ARROW_RIGHT,
        ARROW_LEFT,
        ARROW_DOWN,
        ARROW_UP,
        PAGE_UP,
        PAGE_DOWN,
        HOME,
        END,
        CAPS_LOCK,
        SCROLL_LOCK,
        NUM_LOCK,
        PRINT_SCREEN,
        PAUSE,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,
        NUMPAD_0,
        NUMPAD_1,
        NUMPAD_2,
        NUMPAD_3,
        NUMPAD_4,
        NUMPAD_5,
        NUMPAD_6,
        NUMPAD_7,
        NUMPAD_8,
        NUMPAD_9,
        NUMPAD_DECIMAL,
        NUMPAD_DIVIDE,
        NUMPAD_MULTIPLY,
        NUMPAD_SUBTRACT,
        NUMPAD_ADD,
        NUMPAD_ENTER,
        NUMPAD_EQUAL,
        LEFT_SHIFT,
        LEFT_CONTROL,
        LEFT_ALT,
        LEFT_SUPER,
        RIGHT_SHIFT,
        RIGHT_CONTROL,
        RIGHT_ALT,
        RIGHT_SUPER,
        MENU,
    };

    using ScanCode = int;
    using UnicodeChar = unsigned int;
    
    enum class KeyMods : uint8_t {
        NONE = 0,

        SHIFT     = 1 << 0,
        CONTROL   = 1 << 1,
        ALT       = 1 << 2,
        SUPER     = 1 << 3,
        CAPS_LOCK = 1 << 4,
        NUM_LOCK  = 1 << 5
    };

    constexpr KeyMods operator|(KeyMods lhs, KeyMods rhs) noexcept {
        return static_cast<KeyMods>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    constexpr KeyMods operator|=(KeyMods& lhs, KeyMods rhs) noexcept {
        return lhs = lhs | rhs;
    }

    constexpr bool operator&(KeyMods lhs, KeyMods rhs) noexcept {
        return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
    }

    enum class MouseButton : uint8_t {
        UNKNOWN = 0,

        B1 = 1,
        B2,
        B3,
        B4,
        B5,
        B6,
        B7,
        B8,

        LEFT    = B1,
        RIGHT   = B2,
        MIDDLE  = B3,
        THUMB_1 = B4,
        THUMB_2 = B5,
        EXTRA_1 = B6,
        EXTRA_2 = B7,
    };

    enum class Action : uint8_t {
        UNKNOWN = 0,

        PRESS,
        RELEASE,
        REPEAT
    };

    enum class Joystick : uint8_t {
        UNKNOWN = 0,

        B1,
        B2,
        B3,
        B4,
        B5,
        B6,
        B7,
        B8,
        B9,
        B10,
        B11,
        B12,
        B13,
        B14,
        B15,
        B16,
    };

    enum class GamepadButton : uint8_t {
        UNKNOWN = 0,

        A,
        B,
        X,
        Y,
        LEFT_BUMPER,
        RIGHT_BUMPER,
        BACK,
        START,
        GUIDE,
        LEFT_THUMB,
        RIGHT_THUMB,
        DPAD_UP,
        DPAD_RIGHT,
        DPAD_DOWN,
        DPAD_LEFT,

        CROSS    = A,
        CIRCLE   = B,
        SQUARE   = X,
        TRIANGLE = Y,
    };

    enum class GamepadAxis : uint8_t {
        UNKNOWN = 0,

        LEFT_X,
        LEFT_Y,
        RIGHT_X,
        RIGHT_Y,
        LEFT_TRIGGER,
        RIGHT_TRIGGER,
    };

} // namespace fe::input
