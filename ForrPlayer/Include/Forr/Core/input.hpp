/*===============================================

    Forr Engine

    File : input.hpp
    Role : enum classes of keys, mouse etc

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <cinttypes>

namespace fe::input {
    enum class KeyCode : uint8_t {
        UNKNOWN = 0,

        /* Printable keys */
        KEY_SPACE         = 1 + 0,
        KEY_APOSTROPHE    = 1 + 1, /* ' */
        KEY_COMMA         = 1 + 2, /* , */
        KEY_MINUS         = 1 + 3, /* - */
        KEY_PERIOD        = 1 + 4, /* . */
        KEY_SLASH         = 1 + 5, /* / */
        KEY_0             = 1 + 6,
        KEY_1             = 1 + 7,
        KEY_2             = 1 + 8,
        KEY_3             = 1 + 9,
        KEY_4             = 1 + 10,
        KEY_5             = 1 + 11,
        KEY_6             = 1 + 12,
        KEY_7             = 1 + 13,
        KEY_8             = 1 + 14,
        KEY_9             = 1 + 15,
        KEY_SEMICOLON     = 1 + 16, /* ; */
        KEY_EQUAL         = 1 + 17, /* = */
        KEY_A             = 1 + 18,
        KEY_B             = 1 + 19,
        KEY_C             = 1 + 20,
        KEY_D             = 1 + 21,
        KEY_E             = 1 + 22,
        KEY_F             = 1 + 23,
        KEY_G             = 1 + 24,
        KEY_H             = 1 + 25,
        KEY_I             = 1 + 26,
        KEY_J             = 1 + 27,
        KEY_K             = 1 + 28,
        KEY_L             = 1 + 29,
        KEY_M             = 1 + 30,
        KEY_N             = 1 + 31,
        KEY_O             = 1 + 32,
        KEY_P             = 1 + 33,
        KEY_Q             = 1 + 34,
        KEY_R             = 1 + 35,
        KEY_S             = 1 + 36,
        KEY_T             = 1 + 37,
        KEY_U             = 1 + 38,
        KEY_V             = 1 + 39,
        KEY_W             = 1 + 40,
        KEY_X             = 1 + 41,
        KEY_Y             = 1 + 42,
        KEY_Z             = 1 + 43,
        KEY_LEFT_BRACKET  = 1 + 44, /* [ */
        KEY_BACKSLASH     = 1 + 45, /* \ */
        KEY_RIGHT_BRACKET = 1 + 46, /* ] */
        KEY_GRAVE_ACCENT  = 1 + 47, /* ` */
        KEY_WORLD_1       = 1 + 48, /* non-US #1 */
        KEY_WORLD_2       = 1 + 49, /* non-US #2 */

        /* Function keys */
        KEY_ESCAPE        = 1 + 50,
        KEY_ENTER         = 1 + 51,
        KEY_TAB           = 1 + 52,
        KEY_BACKSPACE     = 1 + 53,
        KEY_INSERT        = 1 + 54,
        KEY_DELETE        = 1 + 55,
        KEY_RIGHT         = 1 + 56,
        KEY_LEFT          = 1 + 57,
        KEY_DOWN          = 1 + 58,
        KEY_UP            = 1 + 59,
        KEY_PAGE_UP       = 1 + 60,
        KEY_PAGE_DOWN     = 1 + 61,
        KEY_HOME          = 1 + 62,
        KEY_END           = 1 + 63,
        KEY_CAPS_LOCK     = 1 + 64,
        KEY_SCROLL_LOCK   = 1 + 65,
        KEY_NUM_LOCK      = 1 + 66,
        KEY_PRINT_SCREEN  = 1 + 67,
        KEY_PAUSE         = 1 + 68,
        KEY_F1            = 1 + 69,
        KEY_F2            = 1 + 70,
        KEY_F3            = 1 + 71,
        KEY_F4            = 1 + 72,
        KEY_F5            = 1 + 73,
        KEY_F6            = 1 + 74,
        KEY_F7            = 1 + 75,
        KEY_F8            = 1 + 76,
        KEY_F9            = 1 + 77,
        KEY_F10           = 1 + 78,
        KEY_F11           = 1 + 79,
        KEY_F12           = 1 + 80,
        KEY_F13           = 1 + 81,
        KEY_F14           = 1 + 82,
        KEY_F15           = 1 + 83,
        KEY_F16           = 1 + 84,
        KEY_F17           = 1 + 85,
        KEY_F18           = 1 + 86,
        KEY_F19           = 1 + 87,
        KEY_F20           = 1 + 88,
        KEY_F21           = 1 + 89,
        KEY_F22           = 1 + 90,
        KEY_F23           = 1 + 91,
        KEY_F24           = 1 + 92,
        KEY_F25           = 1 + 93,
        KEY_KP_0          = 1 + 94,
        KEY_KP_1          = 1 + 95,
        KEY_KP_2          = 1 + 96,
        KEY_KP_3          = 1 + 97,
        KEY_KP_4          = 1 + 98,
        KEY_KP_5          = 1 + 99,
        KEY_KP_6          = 1 + 100,
        KEY_KP_7          = 1 + 101,
        KEY_KP_8          = 1 + 102,
        KEY_KP_9          = 1 + 103,
        KEY_KP_DECIMAL    = 1 + 104,
        KEY_KP_DIVIDE     = 1 + 105,
        KEY_KP_MULTIPLY   = 1 + 106,
        KEY_KP_SUBTRACT   = 1 + 107,
        KEY_KP_ADD        = 1 + 108,
        KEY_KP_ENTER      = 1 + 109,
        KEY_KP_EQUAL      = 1 + 110,
        KEY_LEFT_SHIFT    = 1 + 111,
        KEY_LEFT_CONTROL  = 1 + 112,
        KEY_LEFT_ALT      = 1 + 113,
        KEY_LEFT_SUPER    = 1 + 114,
        KEY_RIGHT_SHIFT   = 1 + 115,
        KEY_RIGHT_CONTROL = 1 + 116,
        KEY_RIGHT_ALT     = 1 + 117,
        KEY_RIGHT_SUPER   = 1 + 118,
        KEY_MENU          = 1 + 119,
    };

    enum class KeyMods : uint8_t {
        UNKNOWN = 0,

        MOD_SHIFT     = 1 << 0,
        MOD_CONTROL   = 1 << 1,
        MOD_ALT       = 1 << 2,
        MOD_SUPER     = 1 << 3,
        MOD_CAPS_LOCK = 1 << 4,
        MOD_NUM_LOCK  = 1 << 5,
    };

    enum class MouseButton : uint8_t {
        UNKNOWN = 0,

        MOUSE_BUTTON_1      = 1 + 0,
        MOUSE_BUTTON_2      = 1 + 1,
        MOUSE_BUTTON_3      = 1 + 2,
        MOUSE_BUTTON_4      = 1 + 3,
        MOUSE_BUTTON_5      = 1 + 4,
        MOUSE_BUTTON_6      = 1 + 5,
        MOUSE_BUTTON_7      = 1 + 6,
        MOUSE_BUTTON_8      = 1 + 7,
        MOUSE_BUTTON_LAST   = MOUSE_BUTTON_8,
        MOUSE_BUTTON_LEFT   = MOUSE_BUTTON_1,
        MOUSE_BUTTON_RIGHT  = MOUSE_BUTTON_2,
        MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3
    };

    enum class Action : uint8_t {
        UNKNOWN = 0,

        PRESS   = 1 + 0,
        RELEASE = 1 + 1,
        REPEAT  = 1 + 2
    };

    enum class Joystick : uint8_t {
        UNKNOWN = 0,

        JOYSTICK_1  = 1 + 0,
        JOYSTICK_2  = 1 + 1,
        JOYSTICK_3  = 1 + 2,
        JOYSTICK_4  = 1 + 3,
        JOYSTICK_5  = 1 + 4,
        JOYSTICK_6  = 1 + 5,
        JOYSTICK_7  = 1 + 6,
        JOYSTICK_8  = 1 + 7,
        JOYSTICK_9  = 1 + 8,
        JOYSTICK_10 = 1 + 9,
        JOYSTICK_11 = 1 + 10,
        JOYSTICK_12 = 1 + 11,
        JOYSTICK_13 = 1 + 12,
        JOYSTICK_14 = 1 + 13,
        JOYSTICK_15 = 1 + 14,
        JOYSTICK_16 = 1 + 15,
    };

    enum class GamepadButton : uint8_t {
        UNKNOWN = 0,

        GAMEPAD_BUTTON_A            = 1 + 0,
        GAMEPAD_BUTTON_B            = 1 + 1,
        GAMEPAD_BUTTON_X            = 1 + 2,
        GAMEPAD_BUTTON_Y            = 1 + 3,
        GAMEPAD_BUTTON_LEFT_BUMPER  = 1 + 4,
        GAMEPAD_BUTTON_RIGHT_BUMPER = 1 + 5,
        GAMEPAD_BUTTON_BACK         = 1 + 6,
        GAMEPAD_BUTTON_START        = 1 + 7,
        GAMEPAD_BUTTON_GUIDE        = 1 + 8,
        GAMEPAD_BUTTON_LEFT_THUMB   = 1 + 9,
        GAMEPAD_BUTTON_RIGHT_THUMB  = 1 + 10,
        GAMEPAD_BUTTON_DPAD_UP      = 1 + 11,
        GAMEPAD_BUTTON_DPAD_RIGHT   = 1 + 12,
        GAMEPAD_BUTTON_DPAD_DOWN    = 1 + 13,
        GAMEPAD_BUTTON_DPAD_LEFT    = 1 + 14,

        GAMEPAD_BUTTON_CROSS    = GAMEPAD_BUTTON_A,
        GAMEPAD_BUTTON_CIRCLE   = GAMEPAD_BUTTON_B,
        GAMEPAD_BUTTON_SQUARE   = GAMEPAD_BUTTON_X,
        GAMEPAD_BUTTON_TRIANGLE = GAMEPAD_BUTTON_Y,
    };

    enum class GamepadAxis : uint8_t {
        UNKNOWN = 0,

        GAMEPAD_AXIS_LEFT_X        = 1 + 0,
        GAMEPAD_AXIS_LEFT_Y        = 1 + 1,
        GAMEPAD_AXIS_RIGHT_X       = 1 + 2,
        GAMEPAD_AXIS_RIGHT_Y       = 1 + 3,
        GAMEPAD_AXIS_LEFT_TRIGGER  = 1 + 4,
        GAMEPAD_AXIS_RIGHT_TRIGGER = 1 + 5,
    };
} // namespace fe::input
