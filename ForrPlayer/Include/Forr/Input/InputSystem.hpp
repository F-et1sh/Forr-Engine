/*===============================================

    Forr Engine

    File : InputSystem.hpp
    Role : InputSystem is a class to work with the game actions

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <variant>
#include <unordered_map>

#include "InputTypes.hpp"
#include "Forr/Core/string.hpp"
#include "Forr/Events/EventBus.hpp"
#include "Forr/Events/Events.hpp"

namespace fe {
    class FORR_API InputSystem {
    public:
        struct ActionState {
            float value{};
            bool  is_pressed{};
            bool  just_pressed{};  // pressed in this frame
            bool  just_released{}; // released in this frame
        };

        struct KeyboardAction {
            input::KeyMods mods{};
            std::variant<input::KeyCode,
                         input::ScanCode,
                         input::UnicodeChar>
                code{};
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

        // @brief call this at the beggining or at the end of the frame in the main loop
        //
        // it resets 'just_pressed' and 'just_released' actions
        void Clear() {
            for (auto& [name, state] : m_Actions) {
                state.just_pressed  = false;
                state.just_released = false;
            }
        }

        template <fe::wstring_like T>
        bool IsActionPressed(fe::hashed_string_base<T> action) const {
            auto it = m_Actions.find(action);
            return it != m_Actions.end() ? it->second.is_pressed : false;
        }

        template <fe::wstring_like T>
        bool IsActionJustPressed(fe::hashed_string_base<T> action) const {
            auto it = m_Actions.find(action);
            return it != m_Actions.end() ? it->second.just_pressed : false;
        }

        template <fe::wstring_like T>
        bool IsActionJustReleased(fe::hashed_string_base<T> action) const {
            auto it = m_Actions.find(action);
            return it != m_Actions.end() ? it->second.just_released : false;
        }

        template <fe::wstring_like T>
        float GetActionValue(fe::hashed_string_base<T> action) const {
            auto it = m_Actions.find(action);
            return it != m_Actions.end() ? it->second.value : 0.0f;
        }

    private:
        void keyboardCallback(const events::KeyButton& key_button);

    private:
        std::reference_wrapper<EventBus> m_EventBus;

        // action name --> is action active
        std::unordered_map<fe::hashed_wstring,
                           ActionState,
                           fe::hashed_wstring_hasher,
                           fe::transparent_equal>
            m_Actions{};
    };
} // namespace fe
