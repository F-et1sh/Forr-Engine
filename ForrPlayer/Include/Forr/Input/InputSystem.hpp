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

    template <typename T>
    concept ActionT = std::is_same_v<T, KeyboardAction> ||
                      std::is_same_v<T, MouseAction> ||
                      std::is_same_v<T, JoystickAction> ||
                      std::is_same_v<T, GamepadButtonAction> ||
                      std::is_same_v<T, GamepadAxisAction>;

    class FORR_API InputSystem {
    public:
        template <ActionT ActionKind>
        struct StoredAction {
            ActionKind      action_data{};
            fe::string_hash action_hashed_name{};
        };

    public:
        InputSystem(EventBus& event_bus);
        ~InputSystem() = default;

        template <ActionT ActionKind>
        void AddAction(const fe::hashed_string_view action_name, const ActionKind& action_data) {
            m_KeyboardActions.push_back({ action_data, action_name.get_hash() });
        }

        // @brief call this at the beggining or at the end of the frame in the main loop
        //
        // it resets 'just_pressed' and 'just_released' actions
        void Clear() {
            for (auto& [name, state] : m_ActionStates) {
                state.just_pressed  = false;
                state.just_released = false;
            }
        }

        bool IsActionPressed(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.is_pressed : false;
        }

        bool IsActionJustPressed(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.just_pressed : false;
        }

        bool IsActionJustReleased(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.just_released : false;
        }

        float GetActionValue(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.value : 0.0f;
        }

    private:
        void keyboardCallback(const event::KeyButton& key_button);
        void unicodeCharTypedCallback(const event::UnicodeCharTyped& unicode_char_typed);

    private:
        std::reference_wrapper<EventBus> m_EventBus;

        // hashed action name --> is action active
        std::unordered_map<fe::string_hash, ActionState> m_ActionStates{};

        std::vector<StoredAction<KeyboardAction>> m_KeyboardActions{};
        // TODO : add more actions
    };
} // namespace fe
