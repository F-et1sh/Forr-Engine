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

namespace fe::input {
    enum class KeyCode : uint8_t {
        /* Printable keys */
        KEY_SPACE         = 0,
        KEY_APOSTROPHE    = 1, /* ' */
        KEY_COMMA         = 2, /* , */
        KEY_MINUS         = 3, /* - */
        KEY_PERIOD        = 4, /* . */
        KEY_SLASH         = 5, /* / */
        KEY_0             = 6,
        KEY_1             = 7,
        KEY_2             = 8,
        KEY_3             = 9,
        KEY_4             = 10,
        KEY_5             = 11,
        KEY_6             = 12,
        KEY_7             = 13,
        KEY_8             = 14,
        KEY_9             = 15,
        KEY_SEMICOLON     = 16, /* ; */
        KEY_EQUAL         = 17, /* = */
        KEY_A             = 18,
        KEY_B             = 19,
        KEY_C             = 20,
        KEY_D             = 21,
        KEY_E             = 22,
        KEY_F             = 23,
        KEY_G             = 24,
        KEY_H             = 25,
        KEY_I             = 26,
        KEY_J             = 27,
        KEY_K             = 28,
        KEY_L             = 29,
        KEY_M             = 30,
        KEY_N             = 31,
        KEY_O             = 32,
        KEY_P             = 33,
        KEY_Q             = 34,
        KEY_R             = 35,
        KEY_S             = 36,
        KEY_T             = 37,
        KEY_U             = 38,
        KEY_V             = 39,
        KEY_W             = 40,
        KEY_X             = 41,
        KEY_Y             = 42,
        KEY_Z             = 43,
        KEY_LEFT_BRACKET  = 44, /* [ */
        KEY_BACKSLASH     = 45, /* \ */
        KEY_RIGHT_BRACKET = 46, /* ] */
        KEY_GRAVE_ACCENT  = 47, /* ` */
        KEY_WORLD_1       = 48, /* non-US #1 */
        KEY_WORLD_2       = 49, /* non-US #2 */

        /* Function keys */
        KEY_ESCAPE        = 50,
        KEY_ENTER         = 51,
        KEY_TAB           = 52,
        KEY_BACKSPACE     = 53,
        KEY_INSERT        = 54,
        KEY_DELETE        = 55,
        KEY_RIGHT         = 56,
        KEY_LEFT          = 57,
        KEY_DOWN          = 58,
        KEY_UP            = 59,
        KEY_PAGE_UP       = 60,
        KEY_PAGE_DOWN     = 61,
        KEY_HOME          = 62,
        KEY_END           = 63,
        KEY_CAPS_LOCK     = 64,
        KEY_SCROLL_LOCK   = 65,
        KEY_NUM_LOCK      = 66,
        KEY_PRINT_SCREEN  = 67,
        KEY_PAUSE         = 68,
        KEY_F1            = 69,
        KEY_F2            = 70,
        KEY_F3            = 71,
        KEY_F4            = 72,
        KEY_F5            = 73,
        KEY_F6            = 74,
        KEY_F7            = 75,
        KEY_F8            = 76,
        KEY_F9            = 77,
        KEY_F10           = 78,
        KEY_F11           = 79,
        KEY_F12           = 80,
        KEY_F13           = 81,
        KEY_F14           = 82,
        KEY_F15           = 83,
        KEY_F16           = 84,
        KEY_F17           = 85,
        KEY_F18           = 86,
        KEY_F19           = 87,
        KEY_F20           = 88,
        KEY_F21           = 89,
        KEY_F22           = 90,
        KEY_F23           = 91,
        KEY_F24           = 92,
        KEY_F25           = 93,
        KEY_KP_0          = 94,
        KEY_KP_1          = 95,
        KEY_KP_2          = 96,
        KEY_KP_3          = 97,
        KEY_KP_4          = 98,
        KEY_KP_5          = 99,
        KEY_KP_6          = 100,
        KEY_KP_7          = 101,
        KEY_KP_8          = 102,
        KEY_KP_9          = 103,
        KEY_KP_DECIMAL    = 104,
        KEY_KP_DIVIDE     = 105,
        KEY_KP_MULTIPLY   = 106,
        KEY_KP_SUBTRACT   = 107,
        KEY_KP_ADD        = 108,
        KEY_KP_ENTER      = 109,
        KEY_KP_EQUAL      = 110,
        KEY_LEFT_SHIFT    = 111,
        KEY_LEFT_CONTROL  = 112,
        KEY_LEFT_ALT      = 113,
        KEY_LEFT_SUPER    = 114,
        KEY_RIGHT_SHIFT   = 115,
        KEY_RIGHT_CONTROL = 116,
        KEY_RIGHT_ALT     = 117,
        KEY_RIGHT_SUPER   = 118,
        KEY_MENU          = 119,
    };

    enum class KeyMods : uint8_t {
        MOD_SHIFT     = 1 << 0,
        MOD_CONTROL   = 1 << 1,
        MOD_ALT       = 1 << 2,
        MOD_SUPER     = 1 << 3,
        MOD_CAPS_LOCK = 1 << 4,
        MOD_NUM_LOCK  = 1 << 5,
    };

    enum class MouseButton : uint8_t {
        MOUSE_BUTTON_1      = 0,
        MOUSE_BUTTON_2      = 1,
        MOUSE_BUTTON_3      = 2,
        MOUSE_BUTTON_4      = 3,
        MOUSE_BUTTON_5      = 4,
        MOUSE_BUTTON_6      = 5,
        MOUSE_BUTTON_7      = 6,
        MOUSE_BUTTON_8      = 7,
        MOUSE_BUTTON_LAST   = MOUSE_BUTTON_8,
        MOUSE_BUTTON_LEFT   = MOUSE_BUTTON_1,
        MOUSE_BUTTON_RIGHT  = MOUSE_BUTTON_2,
        MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3
    };

    enum class InputAction : uint8_t {
        PRESS   = 0,
        RELEASE = 1,
        REPEAT  = 2
    };

    enum class Joystick : uint8_t {
        JOYSTICK_1  = 0,
        JOYSTICK_2  = 1,
        JOYSTICK_3  = 2,
        JOYSTICK_4  = 3,
        JOYSTICK_5  = 4,
        JOYSTICK_6  = 5,
        JOYSTICK_7  = 6,
        JOYSTICK_8  = 7,
        JOYSTICK_9  = 8,
        JOYSTICK_10 = 9,
        JOYSTICK_11 = 10,
        JOYSTICK_12 = 11,
        JOYSTICK_13 = 12,
        JOYSTICK_14 = 13,
        JOYSTICK_15 = 14,
        JOYSTICK_16 = 15,
    };

    enum class GamepadButton : uint8_t {
        GAMEPAD_BUTTON_A            = 0,
        GAMEPAD_BUTTON_B            = 1,
        GAMEPAD_BUTTON_X            = 2,
        GAMEPAD_BUTTON_Y            = 3,
        GAMEPAD_BUTTON_LEFT_BUMPER  = 4,
        GAMEPAD_BUTTON_RIGHT_BUMPER = 5,
        GAMEPAD_BUTTON_BACK         = 6,
        GAMEPAD_BUTTON_START        = 7,
        GAMEPAD_BUTTON_GUIDE        = 8,
        GAMEPAD_BUTTON_LEFT_THUMB   = 9,
        GAMEPAD_BUTTON_RIGHT_THUMB  = 10,
        GAMEPAD_BUTTON_DPAD_UP      = 11,
        GAMEPAD_BUTTON_DPAD_RIGHT   = 12,
        GAMEPAD_BUTTON_DPAD_DOWN    = 13,
        GAMEPAD_BUTTON_DPAD_LEFT    = 14,

        GAMEPAD_BUTTON_CROSS    = GAMEPAD_BUTTON_A,
        GAMEPAD_BUTTON_CIRCLE   = GAMEPAD_BUTTON_B,
        GAMEPAD_BUTTON_SQUARE   = GAMEPAD_BUTTON_X,
        GAMEPAD_BUTTON_TRIANGLE = GAMEPAD_BUTTON_Y,
    };

    enum class GamepadAxis : uint8_t {
        GAMEPAD_AXIS_LEFT_X        = 0,
        GAMEPAD_AXIS_LEFT_Y        = 1,
        GAMEPAD_AXIS_RIGHT_X       = 2,
        GAMEPAD_AXIS_RIGHT_Y       = 3,
        GAMEPAD_AXIS_LEFT_TRIGGER  = 4,
        GAMEPAD_AXIS_RIGHT_TRIGGER = 5,
    };
} // namespace fe::input

namespace fe::events {

    struct KeyEvent {
        fe::input::KeyCode     key{};
        fe::input::InputAction action{};
        uint8_t                mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_NUM_LOCK); }
    };

    struct MouseButtonEvent {
        fe::input::MouseButton button{};
        fe::input::InputAction action{};
        uint8_t                mods{};

        FORR_NODISCARD constexpr inline bool has_shift() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_SHIFT); }
        FORR_NODISCARD constexpr inline bool has_ctrl() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_CONTROL); }
        FORR_NODISCARD constexpr inline bool has_alt() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_ALT); }
        FORR_NODISCARD constexpr inline bool has_super() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_SUPER); }
        FORR_NODISCARD constexpr inline bool has_caps_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_CAPS_LOCK); }
        FORR_NODISCARD constexpr inline bool has_num_lock() const noexcept { return mods & static_cast<uint8_t>(input::KeyMods::MOD_NUM_LOCK); }
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
