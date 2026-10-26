/*===============================================

    Forr Engine

    File : GLFWTranslators.hpp
    Role : GLFW <-> Unified translators

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Core/inputs.hpp"

#include <GLFW/glfw3.h>

namespace fe {
    constexpr static inputs::KeyCode translate_key(int glfw_key) {
        switch (glfw_key) {
                // clang-format off
            case GLFW_KEY_SPACE        : return inputs::KeyCode::KEY_SPACE             ; break;
            case GLFW_KEY_APOSTROPHE   : return inputs::KeyCode::KEY_APOSTROPHE        ; break;
            case GLFW_KEY_COMMA        : return inputs::KeyCode::KEY_COMMA             ; break;
            case GLFW_KEY_MINUS        : return inputs::KeyCode::KEY_MINUS             ; break;
            case GLFW_KEY_PERIOD       : return inputs::KeyCode::KEY_PERIOD            ; break;
            case GLFW_KEY_SLASH        : return inputs::KeyCode::KEY_SLASH             ; break;
            case GLFW_KEY_0            : return inputs::KeyCode::KEY_0                 ; break;
            case GLFW_KEY_1            : return inputs::KeyCode::KEY_1                 ; break;
            case GLFW_KEY_2            : return inputs::KeyCode::KEY_2                 ; break;
            case GLFW_KEY_3            : return inputs::KeyCode::KEY_3                 ; break;
            case GLFW_KEY_4            : return inputs::KeyCode::KEY_4                 ; break;
            case GLFW_KEY_5            : return inputs::KeyCode::KEY_5                 ; break;
            case GLFW_KEY_6            : return inputs::KeyCode::KEY_6                 ; break;
            case GLFW_KEY_7            : return inputs::KeyCode::KEY_7                 ; break;
            case GLFW_KEY_8            : return inputs::KeyCode::KEY_8                 ; break;
            case GLFW_KEY_9            : return inputs::KeyCode::KEY_9                 ; break;
            case GLFW_KEY_SEMICOLON    : return inputs::KeyCode::KEY_SEMICOLON         ; break; 
            case GLFW_KEY_EQUAL        : return inputs::KeyCode::KEY_EQUAL             ; break; 
            case GLFW_KEY_A            : return inputs::KeyCode::KEY_A                 ; break;
            case GLFW_KEY_B            : return inputs::KeyCode::KEY_B                 ; break;
            case GLFW_KEY_C            : return inputs::KeyCode::KEY_C                 ; break;
            case GLFW_KEY_D            : return inputs::KeyCode::KEY_D                 ; break;
            case GLFW_KEY_E            : return inputs::KeyCode::KEY_E                 ; break;
            case GLFW_KEY_F            : return inputs::KeyCode::KEY_F                 ; break;
            case GLFW_KEY_G            : return inputs::KeyCode::KEY_G                 ; break;
            case GLFW_KEY_H            : return inputs::KeyCode::KEY_H                 ; break;
            case GLFW_KEY_I            : return inputs::KeyCode::KEY_I                 ; break;
            case GLFW_KEY_J            : return inputs::KeyCode::KEY_J                 ; break;
            case GLFW_KEY_K            : return inputs::KeyCode::KEY_K                 ; break;
            case GLFW_KEY_L            : return inputs::KeyCode::KEY_L                 ; break;
            case GLFW_KEY_M            : return inputs::KeyCode::KEY_M                 ; break;
            case GLFW_KEY_N            : return inputs::KeyCode::KEY_N                 ; break;
            case GLFW_KEY_O            : return inputs::KeyCode::KEY_O                 ; break;
            case GLFW_KEY_P            : return inputs::KeyCode::KEY_P                 ; break;
            case GLFW_KEY_Q            : return inputs::KeyCode::KEY_Q                 ; break;
            case GLFW_KEY_R            : return inputs::KeyCode::KEY_R                 ; break;
            case GLFW_KEY_S            : return inputs::KeyCode::KEY_S                 ; break;
            case GLFW_KEY_T            : return inputs::KeyCode::KEY_T                 ; break;
            case GLFW_KEY_U            : return inputs::KeyCode::KEY_U                 ; break;
            case GLFW_KEY_V            : return inputs::KeyCode::KEY_V                 ; break;
            case GLFW_KEY_W            : return inputs::KeyCode::KEY_W                 ; break;
            case GLFW_KEY_X            : return inputs::KeyCode::KEY_X                 ; break;
            case GLFW_KEY_Y            : return inputs::KeyCode::KEY_Y                 ; break;
            case GLFW_KEY_Z            : return inputs::KeyCode::KEY_Z                 ; break;
            case GLFW_KEY_LEFT_BRACKET : return inputs::KeyCode::KEY_LEFT_BRACKET      ; break;
            case GLFW_KEY_BACKSLASH    : return inputs::KeyCode::KEY_BACKSLASH         ; break;
            case GLFW_KEY_RIGHT_BRACKET: return inputs::KeyCode::KEY_RIGHT_BRACKET     ; break;
            case GLFW_KEY_GRAVE_ACCENT : return inputs::KeyCode::KEY_GRAVE_ACCENT      ; break;
            case GLFW_KEY_WORLD_1      : return inputs::KeyCode::KEY_WORLD_1           ; break;
            case GLFW_KEY_WORLD_2      : return inputs::KeyCode::KEY_WORLD_2           ; break;

            case GLFW_KEY_ESCAPE       : return inputs::KeyCode::KEY_ESCAPE             ; break;
            case GLFW_KEY_ENTER        : return inputs::KeyCode::KEY_ENTER              ; break;
            case GLFW_KEY_TAB          : return inputs::KeyCode::KEY_TAB                ; break;
            case GLFW_KEY_BACKSPACE    : return inputs::KeyCode::KEY_BACKSPACE          ; break;
            case GLFW_KEY_INSERT       : return inputs::KeyCode::KEY_INSERT             ; break;
            case GLFW_KEY_DELETE       : return inputs::KeyCode::KEY_DELETE             ; break;
            case GLFW_KEY_RIGHT        : return inputs::KeyCode::KEY_RIGHT              ; break;
            case GLFW_KEY_LEFT         : return inputs::KeyCode::KEY_LEFT               ; break;
            case GLFW_KEY_DOWN         : return inputs::KeyCode::KEY_DOWN               ; break;
            case GLFW_KEY_UP           : return inputs::KeyCode::KEY_UP                 ; break;
            case GLFW_KEY_PAGE_UP      : return inputs::KeyCode::KEY_PAGE_UP            ; break;
            case GLFW_KEY_PAGE_DOWN    : return inputs::KeyCode::KEY_PAGE_DOWN          ; break;
            case GLFW_KEY_HOME         : return inputs::KeyCode::KEY_HOME               ; break;
            case GLFW_KEY_END          : return inputs::KeyCode::KEY_END                ; break;
            case GLFW_KEY_CAPS_LOCK    : return inputs::KeyCode::KEY_CAPS_LOCK          ; break;
            case GLFW_KEY_SCROLL_LOCK  : return inputs::KeyCode::KEY_SCROLL_LOCK        ; break;
            case GLFW_KEY_NUM_LOCK     : return inputs::KeyCode::KEY_NUM_LOCK           ; break;
            case GLFW_KEY_PRINT_SCREEN : return inputs::KeyCode::KEY_PRINT_SCREEN       ; break;
            case GLFW_KEY_PAUSE        : return inputs::KeyCode::KEY_PAUSE              ; break;
            case GLFW_KEY_F1           : return inputs::KeyCode::KEY_F1                 ; break;
            case GLFW_KEY_F2           : return inputs::KeyCode::KEY_F2                 ; break;
            case GLFW_KEY_F3           : return inputs::KeyCode::KEY_F3                 ; break;
            case GLFW_KEY_F4           : return inputs::KeyCode::KEY_F4                 ; break;
            case GLFW_KEY_F5           : return inputs::KeyCode::KEY_F5                 ; break;
            case GLFW_KEY_F6           : return inputs::KeyCode::KEY_F6                 ; break;
            case GLFW_KEY_F7           : return inputs::KeyCode::KEY_F7                 ; break;
            case GLFW_KEY_F8           : return inputs::KeyCode::KEY_F8                 ; break;
            case GLFW_KEY_F9           : return inputs::KeyCode::KEY_F9                 ; break;
            case GLFW_KEY_F10          : return inputs::KeyCode::KEY_F10                ; break;
            case GLFW_KEY_F11          : return inputs::KeyCode::KEY_F11                ; break;
            case GLFW_KEY_F12          : return inputs::KeyCode::KEY_F12                ; break;
            case GLFW_KEY_F13          : return inputs::KeyCode::KEY_F13                ; break;
            case GLFW_KEY_F14          : return inputs::KeyCode::KEY_F14                ; break;
            case GLFW_KEY_F15          : return inputs::KeyCode::KEY_F15                ; break;
            case GLFW_KEY_F16          : return inputs::KeyCode::KEY_F16                ; break;
            case GLFW_KEY_F17          : return inputs::KeyCode::KEY_F17                ; break;
            case GLFW_KEY_F18          : return inputs::KeyCode::KEY_F18                ; break;
            case GLFW_KEY_F19          : return inputs::KeyCode::KEY_F19                ; break;
            case GLFW_KEY_F20          : return inputs::KeyCode::KEY_F20                ; break;
            case GLFW_KEY_F21          : return inputs::KeyCode::KEY_F21                ; break;
            case GLFW_KEY_F22          : return inputs::KeyCode::KEY_F22                ; break;
            case GLFW_KEY_F23          : return inputs::KeyCode::KEY_F23                ; break;
            case GLFW_KEY_F24          : return inputs::KeyCode::KEY_F24                ; break;
            case GLFW_KEY_F25          : return inputs::KeyCode::KEY_F25                ; break;
            case GLFW_KEY_KP_0         : return inputs::KeyCode::KEY_KP_0               ; break;
            case GLFW_KEY_KP_1         : return inputs::KeyCode::KEY_KP_1               ; break;
            case GLFW_KEY_KP_2         : return inputs::KeyCode::KEY_KP_2               ; break;
            case GLFW_KEY_KP_3         : return inputs::KeyCode::KEY_KP_3               ; break;
            case GLFW_KEY_KP_4         : return inputs::KeyCode::KEY_KP_4               ; break;
            case GLFW_KEY_KP_5         : return inputs::KeyCode::KEY_KP_5               ; break;
            case GLFW_KEY_KP_6         : return inputs::KeyCode::KEY_KP_6               ; break;
            case GLFW_KEY_KP_7         : return inputs::KeyCode::KEY_KP_7               ; break;
            case GLFW_KEY_KP_8         : return inputs::KeyCode::KEY_KP_8               ; break;
            case GLFW_KEY_KP_9         : return inputs::KeyCode::KEY_KP_9               ; break;
            case GLFW_KEY_KP_DECIMAL   : return inputs::KeyCode::KEY_KP_DECIMAL         ; break;
            case GLFW_KEY_KP_DIVIDE    : return inputs::KeyCode::KEY_KP_DIVIDE          ; break;
            case GLFW_KEY_KP_MULTIPLY  : return inputs::KeyCode::KEY_KP_MULTIPLY        ; break;
            case GLFW_KEY_KP_SUBTRACT  : return inputs::KeyCode::KEY_KP_SUBTRACT        ; break;
            case GLFW_KEY_KP_ADD       : return inputs::KeyCode::KEY_KP_ADD             ; break;
            case GLFW_KEY_KP_ENTER     : return inputs::KeyCode::KEY_KP_ENTER           ; break;
            case GLFW_KEY_KP_EQUAL     : return inputs::KeyCode::KEY_KP_EQUAL           ; break;
            case GLFW_KEY_LEFT_SHIFT   : return inputs::KeyCode::KEY_LEFT_SHIFT         ; break;
            case GLFW_KEY_LEFT_CONTROL : return inputs::KeyCode::KEY_LEFT_CONTROL       ; break;
            case GLFW_KEY_LEFT_ALT     : return inputs::KeyCode::KEY_LEFT_ALT           ; break;
            case GLFW_KEY_LEFT_SUPER   : return inputs::KeyCode::KEY_LEFT_SUPER         ; break;
            case GLFW_KEY_RIGHT_SHIFT  : return inputs::KeyCode::KEY_RIGHT_SHIFT        ; break;
            case GLFW_KEY_RIGHT_CONTROL: return inputs::KeyCode::KEY_RIGHT_CONTROL      ; break;
            case GLFW_KEY_RIGHT_ALT    : return inputs::KeyCode::KEY_RIGHT_ALT          ; break;
            case GLFW_KEY_RIGHT_SUPER  : return inputs::KeyCode::KEY_RIGHT_SUPER        ; break;
            case GLFW_KEY_MENU         : return inputs::KeyCode::KEY_MENU               ; break;
                default                : return inputs::KeyCode::UNKNOWN                ; break;
                // clang-format on
        }
    }

    constexpr static uint8_t translate_mods(int glfw_mods) {
        uint8_t unified_mods = 0;

        if (glfw_mods & GLFW_MOD_SHIFT) unified_mods |= static_cast<uint8_t>(inputs::KeyMods::MOD_SHIFT);
        if (glfw_mods & GLFW_MOD_CONTROL) unified_mods |= static_cast<uint8_t>(inputs::KeyMods::MOD_CONTROL);
        if (glfw_mods & GLFW_MOD_ALT) unified_mods |= static_cast<uint8_t>(inputs::KeyMods::MOD_ALT);
        if (glfw_mods & GLFW_MOD_SUPER) unified_mods |= static_cast<uint8_t>(inputs::KeyMods::MOD_SUPER);
        if (glfw_mods & GLFW_MOD_CAPS_LOCK) unified_mods |= static_cast<uint8_t>(inputs::KeyMods::MOD_CAPS_LOCK);
        if (glfw_mods & GLFW_MOD_NUM_LOCK) unified_mods |= static_cast<uint8_t>(inputs::KeyMods::MOD_NUM_LOCK);

        return unified_mods;
    }

    constexpr static inputs::MouseButton translate_mouse_button(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_MOUSE_BUTTON_1: return inputs::MouseButton::MOUSE_BUTTON_1; break;
            case GLFW_MOUSE_BUTTON_2: return inputs::MouseButton::MOUSE_BUTTON_2; break;
            case GLFW_MOUSE_BUTTON_3: return inputs::MouseButton::MOUSE_BUTTON_3; break;
            case GLFW_MOUSE_BUTTON_4: return inputs::MouseButton::MOUSE_BUTTON_4; break;
            case GLFW_MOUSE_BUTTON_5: return inputs::MouseButton::MOUSE_BUTTON_5; break;
            case GLFW_MOUSE_BUTTON_6: return inputs::MouseButton::MOUSE_BUTTON_6; break;
            case GLFW_MOUSE_BUTTON_7: return inputs::MouseButton::MOUSE_BUTTON_7; break;
            case GLFW_MOUSE_BUTTON_8: return inputs::MouseButton::MOUSE_BUTTON_8; break;
            default                 : return inputs::MouseButton::UNKNOWN       ; break;
            // clang-format on
        }
    }

    constexpr static fe::inputs::InputAction translate_action(int glfw_action) {
        switch (glfw_action) {
                // clang-format off
            case GLFW_RELEASE:return inputs::InputAction::RELEASE; break;
            case GLFW_PRESS  :return inputs::InputAction::PRESS  ; break;
            case GLFW_REPEAT :return inputs::InputAction::REPEAT ; break;
            default          :return inputs::InputAction::UNKNOWN; break;
                // clang-format on
        }
    }

    constexpr static inputs::Joystick translate_joystick(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_JOYSTICK_1 : return inputs::Joystick::JOYSTICK_1 ; break;
            case GLFW_JOYSTICK_2 : return inputs::Joystick::JOYSTICK_2 ; break;
            case GLFW_JOYSTICK_3 : return inputs::Joystick::JOYSTICK_3 ; break;
            case GLFW_JOYSTICK_4 : return inputs::Joystick::JOYSTICK_4 ; break;
            case GLFW_JOYSTICK_5 : return inputs::Joystick::JOYSTICK_5 ; break;
            case GLFW_JOYSTICK_6 : return inputs::Joystick::JOYSTICK_6 ; break;
            case GLFW_JOYSTICK_7 : return inputs::Joystick::JOYSTICK_7 ; break;
            case GLFW_JOYSTICK_8 : return inputs::Joystick::JOYSTICK_8 ; break;
            case GLFW_JOYSTICK_9 : return inputs::Joystick::JOYSTICK_9 ; break;
            case GLFW_JOYSTICK_10: return inputs::Joystick::JOYSTICK_10; break;
            case GLFW_JOYSTICK_11: return inputs::Joystick::JOYSTICK_11; break;
            case GLFW_JOYSTICK_12: return inputs::Joystick::JOYSTICK_12; break;
            case GLFW_JOYSTICK_13: return inputs::Joystick::JOYSTICK_13; break;
            case GLFW_JOYSTICK_14: return inputs::Joystick::JOYSTICK_14; break;
            case GLFW_JOYSTICK_15: return inputs::Joystick::JOYSTICK_15; break;
            case GLFW_JOYSTICK_16: return inputs::Joystick::JOYSTICK_16; break;
            default              : return inputs::Joystick::UNKNOWN    ; break;
            // clang-format on
        }
    }

    constexpr static inputs::GamepadButton translate_gamepad_button(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_GAMEPAD_BUTTON_A           : return inputs::GamepadButton::GAMEPAD_BUTTON_A           ; break;
            case GLFW_GAMEPAD_BUTTON_B           : return inputs::GamepadButton::GAMEPAD_BUTTON_B           ; break;
            case GLFW_GAMEPAD_BUTTON_X           : return inputs::GamepadButton::GAMEPAD_BUTTON_X           ; break;
            case GLFW_GAMEPAD_BUTTON_Y           : return inputs::GamepadButton::GAMEPAD_BUTTON_Y           ; break;
            case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER : return inputs::GamepadButton::GAMEPAD_BUTTON_LEFT_BUMPER ; break;
            case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return inputs::GamepadButton::GAMEPAD_BUTTON_RIGHT_BUMPER; break;
            case GLFW_GAMEPAD_BUTTON_BACK        : return inputs::GamepadButton::GAMEPAD_BUTTON_BACK        ; break;
            case GLFW_GAMEPAD_BUTTON_START       : return inputs::GamepadButton::GAMEPAD_BUTTON_START       ; break;
            case GLFW_GAMEPAD_BUTTON_GUIDE       : return inputs::GamepadButton::GAMEPAD_BUTTON_GUIDE       ; break;
            case GLFW_GAMEPAD_BUTTON_LEFT_THUMB  : return inputs::GamepadButton::GAMEPAD_BUTTON_LEFT_THUMB  ; break;
            case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB : return inputs::GamepadButton::GAMEPAD_BUTTON_RIGHT_THUMB ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_UP     : return inputs::GamepadButton::GAMEPAD_BUTTON_DPAD_UP     ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT  : return inputs::GamepadButton::GAMEPAD_BUTTON_DPAD_RIGHT  ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_DOWN   : return inputs::GamepadButton::GAMEPAD_BUTTON_DPAD_DOWN   ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_LEFT   : return inputs::GamepadButton::GAMEPAD_BUTTON_DPAD_LEFT   ; break;
            default                              : return inputs::GamepadButton::UNKNOWN                    ; break;
            // clang-format on
        }
    }

    constexpr static inputs::GamepadAxis translate_gamepad_axis(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
            // clang-format off
            case GLFW_GAMEPAD_AXIS_LEFT_X        : return inputs::GamepadAxis::GAMEPAD_AXIS_LEFT_X        ; break;
            case GLFW_GAMEPAD_AXIS_LEFT_Y        : return inputs::GamepadAxis::GAMEPAD_AXIS_LEFT_Y        ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_X       : return inputs::GamepadAxis::GAMEPAD_AXIS_RIGHT_X       ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_Y       : return inputs::GamepadAxis::GAMEPAD_AXIS_RIGHT_Y       ; break;
            case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER  : return inputs::GamepadAxis::GAMEPAD_AXIS_LEFT_TRIGGER  ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER : return inputs::GamepadAxis::GAMEPAD_AXIS_RIGHT_TRIGGER ; break;
            default                              : return inputs::GamepadAxis::UNKNOWN                    ; break;
            // clang-format on
        }
    }
} // namespace fe
