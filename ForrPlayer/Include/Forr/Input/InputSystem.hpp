/*===============================================

    Forr Engine

    File : InputSystem.hpp
    Role : InputSystem is a class to work with the game actions

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <unordered_map>

#include "InputTypes.hpp"
#include "Forr/Core/string.hpp"
#include "Forr/Events/EventBus.hpp"

namespace fe {
    class FORR_API InputSystem {
    public:
        struct KeyboardAction {
            input::KeyCode     key{};
            input::KeyMods     mods{};
            input::ScanCode    scan_code{};
            input::UnicodeChar unicode_char{};
        };

        struct MouseAction {
            input::MouseButton button{};
            input::KeyMods     mods{};
        };

        struct JoystickAction {
            input::Joystick button{};
        };

        struct GamepadButtonAction {
            float                dead_zone{};
            input::GamepadButton button{};
        };

        struct GamepadAxisAction {
            float              dead_zone{};
            input::GamepadAxis axis{};
        };

    public:
        InputSystem(EventBus& event_bus);
        ~InputSystem() = default;

        void AddAction() {}
        void IsAction();

    private:
        std::reference_wrapper<EventBus> m_EventBus;

        // action name --> action
        std::unordered_map<fe::hashed_wstring, std::pair<KeyboardAction, input::Action>> m_KeyboardActions{};
    };
} // namespace fe
