/*===============================================

    Forr Engine

    File : GLFWTranslators.hpp
    Role : GLFW <-> Unified translators

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Core/input.hpp"

#include <GLFW/glfw3.h>

namespace fe {
    constexpr static input::KeyCode translate_key(int glfw_key) {
        switch (glfw_key) {
                // clang-format off
            case GLFW_KEY_SPACE        : return input::KeyCode::KEY_SPACE             ; break;
            case GLFW_KEY_APOSTROPHE   : return input::KeyCode::KEY_APOSTROPHE        ; break;
            case GLFW_KEY_COMMA        : return input::KeyCode::KEY_COMMA             ; break;
            case GLFW_KEY_MINUS        : return input::KeyCode::KEY_MINUS             ; break;
            case GLFW_KEY_PERIOD       : return input::KeyCode::KEY_PERIOD            ; break;
            case GLFW_KEY_SLASH        : return input::KeyCode::KEY_SLASH             ; break;
            case GLFW_KEY_0            : return input::KeyCode::KEY_0                 ; break;
            case GLFW_KEY_1            : return input::KeyCode::KEY_1                 ; break;
            case GLFW_KEY_2            : return input::KeyCode::KEY_2                 ; break;
            case GLFW_KEY_3            : return input::KeyCode::KEY_3                 ; break;
            case GLFW_KEY_4            : return input::KeyCode::KEY_4                 ; break;
            case GLFW_KEY_5            : return input::KeyCode::KEY_5                 ; break;
            case GLFW_KEY_6            : return input::KeyCode::KEY_6                 ; break;
            case GLFW_KEY_7            : return input::KeyCode::KEY_7                 ; break;
            case GLFW_KEY_8            : return input::KeyCode::KEY_8                 ; break;
            case GLFW_KEY_9            : return input::KeyCode::KEY_9                 ; break;
            case GLFW_KEY_SEMICOLON    : return input::KeyCode::KEY_SEMICOLON         ; break; 
            case GLFW_KEY_EQUAL        : return input::KeyCode::KEY_EQUAL             ; break; 
            case GLFW_KEY_A            : return input::KeyCode::KEY_A                 ; break;
            case GLFW_KEY_B            : return input::KeyCode::KEY_B                 ; break;
            case GLFW_KEY_C            : return input::KeyCode::KEY_C                 ; break;
            case GLFW_KEY_D            : return input::KeyCode::KEY_D                 ; break;
            case GLFW_KEY_E            : return input::KeyCode::KEY_E                 ; break;
            case GLFW_KEY_F            : return input::KeyCode::KEY_F                 ; break;
            case GLFW_KEY_G            : return input::KeyCode::KEY_G                 ; break;
            case GLFW_KEY_H            : return input::KeyCode::KEY_H                 ; break;
            case GLFW_KEY_I            : return input::KeyCode::KEY_I                 ; break;
            case GLFW_KEY_J            : return input::KeyCode::KEY_J                 ; break;
            case GLFW_KEY_K            : return input::KeyCode::KEY_K                 ; break;
            case GLFW_KEY_L            : return input::KeyCode::KEY_L                 ; break;
            case GLFW_KEY_M            : return input::KeyCode::KEY_M                 ; break;
            case GLFW_KEY_N            : return input::KeyCode::KEY_N                 ; break;
            case GLFW_KEY_O            : return input::KeyCode::KEY_O                 ; break;
            case GLFW_KEY_P            : return input::KeyCode::KEY_P                 ; break;
            case GLFW_KEY_Q            : return input::KeyCode::KEY_Q                 ; break;
            case GLFW_KEY_R            : return input::KeyCode::KEY_R                 ; break;
            case GLFW_KEY_S            : return input::KeyCode::KEY_S                 ; break;
            case GLFW_KEY_T            : return input::KeyCode::KEY_T                 ; break;
            case GLFW_KEY_U            : return input::KeyCode::KEY_U                 ; break;
            case GLFW_KEY_V            : return input::KeyCode::KEY_V                 ; break;
            case GLFW_KEY_W            : return input::KeyCode::KEY_W                 ; break;
            case GLFW_KEY_X            : return input::KeyCode::KEY_X                 ; break;
            case GLFW_KEY_Y            : return input::KeyCode::KEY_Y                 ; break;
            case GLFW_KEY_Z            : return input::KeyCode::KEY_Z                 ; break;
            case GLFW_KEY_LEFT_BRACKET : return input::KeyCode::KEY_LEFT_BRACKET      ; break;
            case GLFW_KEY_BACKSLASH    : return input::KeyCode::KEY_BACKSLASH         ; break;
            case GLFW_KEY_RIGHT_BRACKET: return input::KeyCode::KEY_RIGHT_BRACKET     ; break;
            case GLFW_KEY_GRAVE_ACCENT : return input::KeyCode::KEY_GRAVE_ACCENT      ; break;
            case GLFW_KEY_WORLD_1      : return input::KeyCode::KEY_WORLD_1           ; break;
            case GLFW_KEY_WORLD_2      : return input::KeyCode::KEY_WORLD_2           ; break;

            case GLFW_KEY_ESCAPE       : return input::KeyCode::KEY_ESCAPE             ; break;
            case GLFW_KEY_ENTER        : return input::KeyCode::KEY_ENTER              ; break;
            case GLFW_KEY_TAB          : return input::KeyCode::KEY_TAB                ; break;
            case GLFW_KEY_BACKSPACE    : return input::KeyCode::KEY_BACKSPACE          ; break;
            case GLFW_KEY_INSERT       : return input::KeyCode::KEY_INSERT             ; break;
            case GLFW_KEY_DELETE       : return input::KeyCode::KEY_DELETE             ; break;
            case GLFW_KEY_RIGHT        : return input::KeyCode::KEY_RIGHT              ; break;
            case GLFW_KEY_LEFT         : return input::KeyCode::KEY_LEFT               ; break;
            case GLFW_KEY_DOWN         : return input::KeyCode::KEY_DOWN               ; break;
            case GLFW_KEY_UP           : return input::KeyCode::KEY_UP                 ; break;
            case GLFW_KEY_PAGE_UP      : return input::KeyCode::KEY_PAGE_UP            ; break;
            case GLFW_KEY_PAGE_DOWN    : return input::KeyCode::KEY_PAGE_DOWN          ; break;
            case GLFW_KEY_HOME         : return input::KeyCode::KEY_HOME               ; break;
            case GLFW_KEY_END          : return input::KeyCode::KEY_END                ; break;
            case GLFW_KEY_CAPS_LOCK    : return input::KeyCode::KEY_CAPS_LOCK          ; break;
            case GLFW_KEY_SCROLL_LOCK  : return input::KeyCode::KEY_SCROLL_LOCK        ; break;
            case GLFW_KEY_NUM_LOCK     : return input::KeyCode::KEY_NUM_LOCK           ; break;
            case GLFW_KEY_PRINT_SCREEN : return input::KeyCode::KEY_PRINT_SCREEN       ; break;
            case GLFW_KEY_PAUSE        : return input::KeyCode::KEY_PAUSE              ; break;
            case GLFW_KEY_F1           : return input::KeyCode::KEY_F1                 ; break;
            case GLFW_KEY_F2           : return input::KeyCode::KEY_F2                 ; break;
            case GLFW_KEY_F3           : return input::KeyCode::KEY_F3                 ; break;
            case GLFW_KEY_F4           : return input::KeyCode::KEY_F4                 ; break;
            case GLFW_KEY_F5           : return input::KeyCode::KEY_F5                 ; break;
            case GLFW_KEY_F6           : return input::KeyCode::KEY_F6                 ; break;
            case GLFW_KEY_F7           : return input::KeyCode::KEY_F7                 ; break;
            case GLFW_KEY_F8           : return input::KeyCode::KEY_F8                 ; break;
            case GLFW_KEY_F9           : return input::KeyCode::KEY_F9                 ; break;
            case GLFW_KEY_F10          : return input::KeyCode::KEY_F10                ; break;
            case GLFW_KEY_F11          : return input::KeyCode::KEY_F11                ; break;
            case GLFW_KEY_F12          : return input::KeyCode::KEY_F12                ; break;
            case GLFW_KEY_F13          : return input::KeyCode::KEY_F13                ; break;
            case GLFW_KEY_F14          : return input::KeyCode::KEY_F14                ; break;
            case GLFW_KEY_F15          : return input::KeyCode::KEY_F15                ; break;
            case GLFW_KEY_F16          : return input::KeyCode::KEY_F16                ; break;
            case GLFW_KEY_F17          : return input::KeyCode::KEY_F17                ; break;
            case GLFW_KEY_F18          : return input::KeyCode::KEY_F18                ; break;
            case GLFW_KEY_F19          : return input::KeyCode::KEY_F19                ; break;
            case GLFW_KEY_F20          : return input::KeyCode::KEY_F20                ; break;
            case GLFW_KEY_F21          : return input::KeyCode::KEY_F21                ; break;
            case GLFW_KEY_F22          : return input::KeyCode::KEY_F22                ; break;
            case GLFW_KEY_F23          : return input::KeyCode::KEY_F23                ; break;
            case GLFW_KEY_F24          : return input::KeyCode::KEY_F24                ; break;
            case GLFW_KEY_F25          : return input::KeyCode::KEY_F25                ; break;
            case GLFW_KEY_KP_0         : return input::KeyCode::KEY_KP_0               ; break;
            case GLFW_KEY_KP_1         : return input::KeyCode::KEY_KP_1               ; break;
            case GLFW_KEY_KP_2         : return input::KeyCode::KEY_KP_2               ; break;
            case GLFW_KEY_KP_3         : return input::KeyCode::KEY_KP_3               ; break;
            case GLFW_KEY_KP_4         : return input::KeyCode::KEY_KP_4               ; break;
            case GLFW_KEY_KP_5         : return input::KeyCode::KEY_KP_5               ; break;
            case GLFW_KEY_KP_6         : return input::KeyCode::KEY_KP_6               ; break;
            case GLFW_KEY_KP_7         : return input::KeyCode::KEY_KP_7               ; break;
            case GLFW_KEY_KP_8         : return input::KeyCode::KEY_KP_8               ; break;
            case GLFW_KEY_KP_9         : return input::KeyCode::KEY_KP_9               ; break;
            case GLFW_KEY_KP_DECIMAL   : return input::KeyCode::KEY_KP_DECIMAL         ; break;
            case GLFW_KEY_KP_DIVIDE    : return input::KeyCode::KEY_KP_DIVIDE          ; break;
            case GLFW_KEY_KP_MULTIPLY  : return input::KeyCode::KEY_KP_MULTIPLY        ; break;
            case GLFW_KEY_KP_SUBTRACT  : return input::KeyCode::KEY_KP_SUBTRACT        ; break;
            case GLFW_KEY_KP_ADD       : return input::KeyCode::KEY_KP_ADD             ; break;
            case GLFW_KEY_KP_ENTER     : return input::KeyCode::KEY_KP_ENTER           ; break;
            case GLFW_KEY_KP_EQUAL     : return input::KeyCode::KEY_KP_EQUAL           ; break;
            case GLFW_KEY_LEFT_SHIFT   : return input::KeyCode::KEY_LEFT_SHIFT         ; break;
            case GLFW_KEY_LEFT_CONTROL : return input::KeyCode::KEY_LEFT_CONTROL       ; break;
            case GLFW_KEY_LEFT_ALT     : return input::KeyCode::KEY_LEFT_ALT           ; break;
            case GLFW_KEY_LEFT_SUPER   : return input::KeyCode::KEY_LEFT_SUPER         ; break;
            case GLFW_KEY_RIGHT_SHIFT  : return input::KeyCode::KEY_RIGHT_SHIFT        ; break;
            case GLFW_KEY_RIGHT_CONTROL: return input::KeyCode::KEY_RIGHT_CONTROL      ; break;
            case GLFW_KEY_RIGHT_ALT    : return input::KeyCode::KEY_RIGHT_ALT          ; break;
            case GLFW_KEY_RIGHT_SUPER  : return input::KeyCode::KEY_RIGHT_SUPER        ; break;
            case GLFW_KEY_MENU         : return input::KeyCode::KEY_MENU               ; break;
                default                : return input::KeyCode::UNKNOWN                ; break;
                // clang-format on
        }
    }

    constexpr static uint8_t translate_mods(int glfw_mods) {
        uint8_t unified_mods = 0;

        if (glfw_mods & GLFW_MOD_SHIFT) unified_mods |= static_cast<uint8_t>(input::KeyMods::MOD_SHIFT);
        if (glfw_mods & GLFW_MOD_CONTROL) unified_mods |= static_cast<uint8_t>(input::KeyMods::MOD_CONTROL);
        if (glfw_mods & GLFW_MOD_ALT) unified_mods |= static_cast<uint8_t>(input::KeyMods::MOD_ALT);
        if (glfw_mods & GLFW_MOD_SUPER) unified_mods |= static_cast<uint8_t>(input::KeyMods::MOD_SUPER);
        if (glfw_mods & GLFW_MOD_CAPS_LOCK) unified_mods |= static_cast<uint8_t>(input::KeyMods::MOD_CAPS_LOCK);
        if (glfw_mods & GLFW_MOD_NUM_LOCK) unified_mods |= static_cast<uint8_t>(input::KeyMods::MOD_NUM_LOCK);

        return unified_mods;
    }

    constexpr static input::MouseButton translate_mouse_button(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_MOUSE_BUTTON_1: return input::MouseButton::MOUSE_BUTTON_1; break;
            case GLFW_MOUSE_BUTTON_2: return input::MouseButton::MOUSE_BUTTON_2; break;
            case GLFW_MOUSE_BUTTON_3: return input::MouseButton::MOUSE_BUTTON_3; break;
            case GLFW_MOUSE_BUTTON_4: return input::MouseButton::MOUSE_BUTTON_4; break;
            case GLFW_MOUSE_BUTTON_5: return input::MouseButton::MOUSE_BUTTON_5; break;
            case GLFW_MOUSE_BUTTON_6: return input::MouseButton::MOUSE_BUTTON_6; break;
            case GLFW_MOUSE_BUTTON_7: return input::MouseButton::MOUSE_BUTTON_7; break;
            case GLFW_MOUSE_BUTTON_8: return input::MouseButton::MOUSE_BUTTON_8; break;
            default                 : return input::MouseButton::UNKNOWN       ; break;
            // clang-format on
        }
    }

    constexpr static input::Action translate_action(int glfw_action) {
        switch (glfw_action) {
                // clang-format off
            case GLFW_RELEASE:return input::Action::RELEASE; break;
            case GLFW_PRESS  :return input::Action::PRESS  ; break;
            case GLFW_REPEAT :return input::Action::REPEAT ; break;
            default          :return input::Action::UNKNOWN; break;
                // clang-format on
        }
    }

    constexpr static input::Joystick translate_joystick(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_JOYSTICK_1 : return input::Joystick::JOYSTICK_1 ; break;
            case GLFW_JOYSTICK_2 : return input::Joystick::JOYSTICK_2 ; break;
            case GLFW_JOYSTICK_3 : return input::Joystick::JOYSTICK_3 ; break;
            case GLFW_JOYSTICK_4 : return input::Joystick::JOYSTICK_4 ; break;
            case GLFW_JOYSTICK_5 : return input::Joystick::JOYSTICK_5 ; break;
            case GLFW_JOYSTICK_6 : return input::Joystick::JOYSTICK_6 ; break;
            case GLFW_JOYSTICK_7 : return input::Joystick::JOYSTICK_7 ; break;
            case GLFW_JOYSTICK_8 : return input::Joystick::JOYSTICK_8 ; break;
            case GLFW_JOYSTICK_9 : return input::Joystick::JOYSTICK_9 ; break;
            case GLFW_JOYSTICK_10: return input::Joystick::JOYSTICK_10; break;
            case GLFW_JOYSTICK_11: return input::Joystick::JOYSTICK_11; break;
            case GLFW_JOYSTICK_12: return input::Joystick::JOYSTICK_12; break;
            case GLFW_JOYSTICK_13: return input::Joystick::JOYSTICK_13; break;
            case GLFW_JOYSTICK_14: return input::Joystick::JOYSTICK_14; break;
            case GLFW_JOYSTICK_15: return input::Joystick::JOYSTICK_15; break;
            case GLFW_JOYSTICK_16: return input::Joystick::JOYSTICK_16; break;
            default              : return input::Joystick::UNKNOWN    ; break;
            // clang-format on
        }
    }

    constexpr static input::GamepadButton translate_gamepad_button(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_GAMEPAD_BUTTON_A           : return input::GamepadButton::GAMEPAD_BUTTON_A           ; break;
            case GLFW_GAMEPAD_BUTTON_B           : return input::GamepadButton::GAMEPAD_BUTTON_B           ; break;
            case GLFW_GAMEPAD_BUTTON_X           : return input::GamepadButton::GAMEPAD_BUTTON_X           ; break;
            case GLFW_GAMEPAD_BUTTON_Y           : return input::GamepadButton::GAMEPAD_BUTTON_Y           ; break;
            case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER : return input::GamepadButton::GAMEPAD_BUTTON_LEFT_BUMPER ; break;
            case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return input::GamepadButton::GAMEPAD_BUTTON_RIGHT_BUMPER; break;
            case GLFW_GAMEPAD_BUTTON_BACK        : return input::GamepadButton::GAMEPAD_BUTTON_BACK        ; break;
            case GLFW_GAMEPAD_BUTTON_START       : return input::GamepadButton::GAMEPAD_BUTTON_START       ; break;
            case GLFW_GAMEPAD_BUTTON_GUIDE       : return input::GamepadButton::GAMEPAD_BUTTON_GUIDE       ; break;
            case GLFW_GAMEPAD_BUTTON_LEFT_THUMB  : return input::GamepadButton::GAMEPAD_BUTTON_LEFT_THUMB  ; break;
            case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB : return input::GamepadButton::GAMEPAD_BUTTON_RIGHT_THUMB ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_UP     : return input::GamepadButton::GAMEPAD_BUTTON_DPAD_UP     ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT  : return input::GamepadButton::GAMEPAD_BUTTON_DPAD_RIGHT  ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_DOWN   : return input::GamepadButton::GAMEPAD_BUTTON_DPAD_DOWN   ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_LEFT   : return input::GamepadButton::GAMEPAD_BUTTON_DPAD_LEFT   ; break;
            default                              : return input::GamepadButton::UNKNOWN                    ; break;
            // clang-format on
        }
    }

    constexpr static input::GamepadAxis translate_gamepad_axis(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_GAMEPAD_AXIS_LEFT_X        : return input::GamepadAxis::GAMEPAD_AXIS_LEFT_X        ; break;
            case GLFW_GAMEPAD_AXIS_LEFT_Y        : return input::GamepadAxis::GAMEPAD_AXIS_LEFT_Y        ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_X       : return input::GamepadAxis::GAMEPAD_AXIS_RIGHT_X       ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_Y       : return input::GamepadAxis::GAMEPAD_AXIS_RIGHT_Y       ; break;
            case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER  : return input::GamepadAxis::GAMEPAD_AXIS_LEFT_TRIGGER  ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER : return input::GamepadAxis::GAMEPAD_AXIS_RIGHT_TRIGGER ; break;
            default                              : return input::GamepadAxis::UNKNOWN                    ; break;
            // clang-format on
        }
    }
} // namespace fe
