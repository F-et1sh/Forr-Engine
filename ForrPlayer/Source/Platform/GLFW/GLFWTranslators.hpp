/*===============================================

    Forr Engine

    File : GLFWTranslators.hpp
    Role : GLFW <-> Unified translators

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Forr/Input/InputTypes.hpp"

#include <GLFW/glfw3.h>

namespace fe {
    constexpr static input::KeyCode translate_key(int glfw_key) {
        switch (glfw_key) {
                // clang-format off
            case GLFW_KEY_SPACE        : return input::KeyCode::SPACE          ; break;
            case GLFW_KEY_APOSTROPHE   : return input::KeyCode::APOSTROPHE     ; break;
            case GLFW_KEY_COMMA        : return input::KeyCode::COMMA          ; break;
            case GLFW_KEY_MINUS        : return input::KeyCode::MINUS          ; break;
            case GLFW_KEY_PERIOD       : return input::KeyCode::PERIOD         ; break;
            case GLFW_KEY_SLASH        : return input::KeyCode::SLASH          ; break;
            case GLFW_KEY_0            : return input::KeyCode::NUMBER_0       ; break;
            case GLFW_KEY_1            : return input::KeyCode::NUMBER_1       ; break;
            case GLFW_KEY_2            : return input::KeyCode::NUMBER_2       ; break;
            case GLFW_KEY_3            : return input::KeyCode::NUMBER_3       ; break;
            case GLFW_KEY_4            : return input::KeyCode::NUMBER_4       ; break;
            case GLFW_KEY_5            : return input::KeyCode::NUMBER_5       ; break;
            case GLFW_KEY_6            : return input::KeyCode::NUMBER_6       ; break;
            case GLFW_KEY_7            : return input::KeyCode::NUMBER_7       ; break;
            case GLFW_KEY_8            : return input::KeyCode::NUMBER_8       ; break;
            case GLFW_KEY_9            : return input::KeyCode::NUMBER_9       ; break;
            case GLFW_KEY_SEMICOLON    : return input::KeyCode::SEMICOLON      ; break; 
            case GLFW_KEY_EQUAL        : return input::KeyCode::EQUAL          ; break; 
            case GLFW_KEY_A            : return input::KeyCode::A              ; break;
            case GLFW_KEY_B            : return input::KeyCode::B              ; break;
            case GLFW_KEY_C            : return input::KeyCode::C              ; break;
            case GLFW_KEY_D            : return input::KeyCode::D              ; break;
            case GLFW_KEY_E            : return input::KeyCode::E              ; break;
            case GLFW_KEY_F            : return input::KeyCode::F              ; break;
            case GLFW_KEY_G            : return input::KeyCode::G              ; break;
            case GLFW_KEY_H            : return input::KeyCode::H              ; break;
            case GLFW_KEY_I            : return input::KeyCode::I              ; break;
            case GLFW_KEY_J            : return input::KeyCode::J              ; break;
            case GLFW_KEY_K            : return input::KeyCode::K              ; break;
            case GLFW_KEY_L            : return input::KeyCode::L              ; break;
            case GLFW_KEY_M            : return input::KeyCode::M              ; break;
            case GLFW_KEY_N            : return input::KeyCode::N              ; break;
            case GLFW_KEY_O            : return input::KeyCode::O              ; break;
            case GLFW_KEY_P            : return input::KeyCode::P              ; break;
            case GLFW_KEY_Q            : return input::KeyCode::Q              ; break;
            case GLFW_KEY_R            : return input::KeyCode::R              ; break;
            case GLFW_KEY_S            : return input::KeyCode::S              ; break;
            case GLFW_KEY_T            : return input::KeyCode::T              ; break;
            case GLFW_KEY_U            : return input::KeyCode::U              ; break;
            case GLFW_KEY_V            : return input::KeyCode::V              ; break;
            case GLFW_KEY_W            : return input::KeyCode::W              ; break;
            case GLFW_KEY_X            : return input::KeyCode::X              ; break;
            case GLFW_KEY_Y            : return input::KeyCode::Y              ; break;
            case GLFW_KEY_Z            : return input::KeyCode::Z              ; break;
            case GLFW_KEY_LEFT_BRACKET : return input::KeyCode::LEFT_BRACKET   ; break;
            case GLFW_KEY_BACKSLASH    : return input::KeyCode::BACKSLASH      ; break;
            case GLFW_KEY_RIGHT_BRACKET: return input::KeyCode::RIGHT_BRACKET  ; break;
            case GLFW_KEY_GRAVE_ACCENT : return input::KeyCode::GRAVE_ACCENT   ; break;
            case GLFW_KEY_WORLD_1      : return input::KeyCode::WORLD_1        ; break;
            case GLFW_KEY_WORLD_2      : return input::KeyCode::WORLD_2        ; break;

            case GLFW_KEY_ESCAPE       : return input::KeyCode::ESCAPE         ; break;
            case GLFW_KEY_ENTER        : return input::KeyCode::ENTER          ; break;
            case GLFW_KEY_TAB          : return input::KeyCode::TAB            ; break;
            case GLFW_KEY_BACKSPACE    : return input::KeyCode::BACKSPACE      ; break;
            case GLFW_KEY_INSERT       : return input::KeyCode::INSERT         ; break;
            case GLFW_KEY_DELETE       : return input::KeyCode::DELETE         ; break;
            case GLFW_KEY_RIGHT        : return input::KeyCode::ARROW_RIGHT    ; break;
            case GLFW_KEY_LEFT         : return input::KeyCode::ARROW_LEFT     ; break;
            case GLFW_KEY_DOWN         : return input::KeyCode::ARROW_DOWN     ; break;
            case GLFW_KEY_UP           : return input::KeyCode::ARROW_UP       ; break;
            case GLFW_KEY_PAGE_UP      : return input::KeyCode::PAGE_UP        ; break;
            case GLFW_KEY_PAGE_DOWN    : return input::KeyCode::PAGE_DOWN      ; break;
            case GLFW_KEY_HOME         : return input::KeyCode::HOME           ; break;
            case GLFW_KEY_END          : return input::KeyCode::END            ; break;
            case GLFW_KEY_CAPS_LOCK    : return input::KeyCode::CAPS_LOCK      ; break;
            case GLFW_KEY_SCROLL_LOCK  : return input::KeyCode::SCROLL_LOCK    ; break;
            case GLFW_KEY_NUM_LOCK     : return input::KeyCode::NUM_LOCK       ; break;
            case GLFW_KEY_PRINT_SCREEN : return input::KeyCode::PRINT_SCREEN   ; break;
            case GLFW_KEY_PAUSE        : return input::KeyCode::PAUSE          ; break;
            case GLFW_KEY_F1           : return input::KeyCode::F1             ; break;
            case GLFW_KEY_F2           : return input::KeyCode::F2             ; break;
            case GLFW_KEY_F3           : return input::KeyCode::F3             ; break;
            case GLFW_KEY_F4           : return input::KeyCode::F4             ; break;
            case GLFW_KEY_F5           : return input::KeyCode::F5             ; break;
            case GLFW_KEY_F6           : return input::KeyCode::F6             ; break;
            case GLFW_KEY_F7           : return input::KeyCode::F7             ; break;
            case GLFW_KEY_F8           : return input::KeyCode::F8             ; break;
            case GLFW_KEY_F9           : return input::KeyCode::F9             ; break;
            case GLFW_KEY_F10          : return input::KeyCode::F10            ; break;
            case GLFW_KEY_F11          : return input::KeyCode::F11            ; break;
            case GLFW_KEY_F12          : return input::KeyCode::F12            ; break;
            case GLFW_KEY_F13          : return input::KeyCode::F13            ; break;
            case GLFW_KEY_F14          : return input::KeyCode::F14            ; break;
            case GLFW_KEY_F15          : return input::KeyCode::F15            ; break;
            case GLFW_KEY_F16          : return input::KeyCode::F16            ; break;
            case GLFW_KEY_F17          : return input::KeyCode::F17            ; break;
            case GLFW_KEY_F18          : return input::KeyCode::F18            ; break;
            case GLFW_KEY_F19          : return input::KeyCode::F19            ; break;
            case GLFW_KEY_F20          : return input::KeyCode::F20            ; break;
            case GLFW_KEY_F21          : return input::KeyCode::F21            ; break;
            case GLFW_KEY_F22          : return input::KeyCode::F22            ; break;
            case GLFW_KEY_F23          : return input::KeyCode::F23            ; break;
            case GLFW_KEY_F24          : return input::KeyCode::F24            ; break;
            case GLFW_KEY_F25          : return input::KeyCode::F25            ; break;
            case GLFW_KEY_KP_0         : return input::KeyCode::NUMPAD_0       ; break;
            case GLFW_KEY_KP_1         : return input::KeyCode::NUMPAD_1       ; break;
            case GLFW_KEY_KP_2         : return input::KeyCode::NUMPAD_2       ; break;
            case GLFW_KEY_KP_3         : return input::KeyCode::NUMPAD_3       ; break;
            case GLFW_KEY_KP_4         : return input::KeyCode::NUMPAD_4       ; break;
            case GLFW_KEY_KP_5         : return input::KeyCode::NUMPAD_5       ; break;
            case GLFW_KEY_KP_6         : return input::KeyCode::NUMPAD_6       ; break;
            case GLFW_KEY_KP_7         : return input::KeyCode::NUMPAD_7       ; break;
            case GLFW_KEY_KP_8         : return input::KeyCode::NUMPAD_8       ; break;
            case GLFW_KEY_KP_9         : return input::KeyCode::NUMPAD_9       ; break;
            case GLFW_KEY_KP_DECIMAL   : return input::KeyCode::NUMPAD_DECIMAL ; break;
            case GLFW_KEY_KP_DIVIDE    : return input::KeyCode::NUMPAD_DIVIDE  ; break;
            case GLFW_KEY_KP_MULTIPLY  : return input::KeyCode::NUMPAD_MULTIPLY; break;
            case GLFW_KEY_KP_SUBTRACT  : return input::KeyCode::NUMPAD_SUBTRACT; break;
            case GLFW_KEY_KP_ADD       : return input::KeyCode::NUMPAD_ADD     ; break;
            case GLFW_KEY_KP_ENTER     : return input::KeyCode::NUMPAD_ENTER   ; break;
            case GLFW_KEY_KP_EQUAL     : return input::KeyCode::NUMPAD_EQUAL   ; break;
            case GLFW_KEY_LEFT_SHIFT   : return input::KeyCode::LEFT_SHIFT     ; break;
            case GLFW_KEY_LEFT_CONTROL : return input::KeyCode::LEFT_CONTROL   ; break;
            case GLFW_KEY_LEFT_ALT     : return input::KeyCode::LEFT_ALT       ; break;
            case GLFW_KEY_LEFT_SUPER   : return input::KeyCode::LEFT_SUPER     ; break;
            case GLFW_KEY_RIGHT_SHIFT  : return input::KeyCode::RIGHT_SHIFT    ; break;
            case GLFW_KEY_RIGHT_CONTROL: return input::KeyCode::RIGHT_CONTROL  ; break;
            case GLFW_KEY_RIGHT_ALT    : return input::KeyCode::RIGHT_ALT      ; break;
            case GLFW_KEY_RIGHT_SUPER  : return input::KeyCode::RIGHT_SUPER    ; break;
            case GLFW_KEY_MENU         : return input::KeyCode::MENU           ; break;
                default                : return input::KeyCode::UNKNOWN        ; break;
                // clang-format on
        }
    }

    constexpr static uint8_t translate_mods(int glfw_mods) {
        uint8_t unified_mods = 0;

        if (glfw_mods & GLFW_MOD_SHIFT) unified_mods |= static_cast<uint8_t>(input::KeyMods::SHIFT);
        if (glfw_mods & GLFW_MOD_CONTROL) unified_mods |= static_cast<uint8_t>(input::KeyMods::CONTROL);
        if (glfw_mods & GLFW_MOD_ALT) unified_mods |= static_cast<uint8_t>(input::KeyMods::ALT);
        if (glfw_mods & GLFW_MOD_SUPER) unified_mods |= static_cast<uint8_t>(input::KeyMods::SUPER);
        if (glfw_mods & GLFW_MOD_CAPS_LOCK) unified_mods |= static_cast<uint8_t>(input::KeyMods::CAPS_LOCK);
        if (glfw_mods & GLFW_MOD_NUM_LOCK) unified_mods |= static_cast<uint8_t>(input::KeyMods::NUM_LOCK);

        return unified_mods;
    }

    constexpr static input::MouseButton translate_mouse_button(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
                // clang-format off
            case GLFW_MOUSE_BUTTON_1: return input::MouseButton::B1     ; break;
            case GLFW_MOUSE_BUTTON_2: return input::MouseButton::B2     ; break;
            case GLFW_MOUSE_BUTTON_3: return input::MouseButton::B3     ; break;
            case GLFW_MOUSE_BUTTON_4: return input::MouseButton::B4     ; break;
            case GLFW_MOUSE_BUTTON_5: return input::MouseButton::B5     ; break;
            case GLFW_MOUSE_BUTTON_6: return input::MouseButton::B6     ; break;
            case GLFW_MOUSE_BUTTON_7: return input::MouseButton::B7     ; break;
            case GLFW_MOUSE_BUTTON_8: return input::MouseButton::B8     ; break;
            default                 : return input::MouseButton::UNKNOWN; break;
                // clang-format on
        }
    }

    constexpr static input::Action translate_action(int glfw_action) {
        switch (glfw_action) {
                // clang-format off
            case GLFW_RELEASE: return input::Action::RELEASE; break;
            case GLFW_PRESS  : return input::Action::PRESS  ; break;
            case GLFW_REPEAT : return input::Action::REPEAT ; break;
            default          : return input::Action::UNKNOWN; break;
                // clang-format on
        }
    }

    constexpr static input::Joystick translate_joystick(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
                // clang-format off
            case GLFW_JOYSTICK_1 : return input::Joystick::B1     ; break;
            case GLFW_JOYSTICK_2 : return input::Joystick::B2     ; break;
            case GLFW_JOYSTICK_3 : return input::Joystick::B3     ; break;
            case GLFW_JOYSTICK_4 : return input::Joystick::B4     ; break;
            case GLFW_JOYSTICK_5 : return input::Joystick::B5     ; break;
            case GLFW_JOYSTICK_6 : return input::Joystick::B6     ; break;
            case GLFW_JOYSTICK_7 : return input::Joystick::B7     ; break;
            case GLFW_JOYSTICK_8 : return input::Joystick::B8     ; break;
            case GLFW_JOYSTICK_9 : return input::Joystick::B9     ; break;
            case GLFW_JOYSTICK_10: return input::Joystick::B10    ; break;
            case GLFW_JOYSTICK_11: return input::Joystick::B11    ; break;
            case GLFW_JOYSTICK_12: return input::Joystick::B12    ; break;
            case GLFW_JOYSTICK_13: return input::Joystick::B13    ; break;
            case GLFW_JOYSTICK_14: return input::Joystick::B14    ; break;
            case GLFW_JOYSTICK_15: return input::Joystick::B15    ; break;
            case GLFW_JOYSTICK_16: return input::Joystick::B16    ; break;
            default              : return input::Joystick::UNKNOWN; break;
                // clang-format on
        }
    }

    constexpr static input::GamepadButton translate_gamepad_button(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
                // clang-format off
            case GLFW_GAMEPAD_BUTTON_A           : return input::GamepadButton::A           ; break;
            case GLFW_GAMEPAD_BUTTON_B           : return input::GamepadButton::B           ; break;
            case GLFW_GAMEPAD_BUTTON_X           : return input::GamepadButton::X           ; break;
            case GLFW_GAMEPAD_BUTTON_Y           : return input::GamepadButton::Y           ; break;
            case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER : return input::GamepadButton::LEFT_BUMPER ; break;
            case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return input::GamepadButton::RIGHT_BUMPER; break;
            case GLFW_GAMEPAD_BUTTON_BACK        : return input::GamepadButton::BACK        ; break;
            case GLFW_GAMEPAD_BUTTON_START       : return input::GamepadButton::START       ; break;
            case GLFW_GAMEPAD_BUTTON_GUIDE       : return input::GamepadButton::GUIDE       ; break;
            case GLFW_GAMEPAD_BUTTON_LEFT_THUMB  : return input::GamepadButton::LEFT_THUMB  ; break;
            case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB : return input::GamepadButton::RIGHT_THUMB ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_UP     : return input::GamepadButton::DPAD_UP     ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT  : return input::GamepadButton::DPAD_RIGHT  ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_DOWN   : return input::GamepadButton::DPAD_DOWN   ; break;
            case GLFW_GAMEPAD_BUTTON_DPAD_LEFT   : return input::GamepadButton::DPAD_LEFT   ; break;
            default                              : return input::GamepadButton::UNKNOWN     ; break;
                // clang-format on
        }
    }

    constexpr static input::GamepadAxis translate_gamepad_axis(int glfw_mouse_button) {
        switch (glfw_mouse_button) {
                // clang-format off
            case GLFW_GAMEPAD_AXIS_LEFT_X        : return input::GamepadAxis::LEFT_X        ; break;
            case GLFW_GAMEPAD_AXIS_LEFT_Y        : return input::GamepadAxis::LEFT_Y        ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_X       : return input::GamepadAxis::RIGHT_X       ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_Y       : return input::GamepadAxis::RIGHT_Y       ; break;
            case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER  : return input::GamepadAxis::LEFT_TRIGGER  ; break;
            case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER : return input::GamepadAxis::RIGHT_TRIGGER ; break;
            default                              : return input::GamepadAxis::UNKNOWN       ; break;
                // clang-format on
        }
    }
} // namespace fe
