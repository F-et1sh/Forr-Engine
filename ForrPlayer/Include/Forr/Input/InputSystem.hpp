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

    //struct JoystickAction {
    //    input::Joystick button{};
    //};
    //
    //struct GamepadButtonAction {
    //    float                dead_zone{};
    //    input::GamepadButton button{};
    //};
    //
    //struct GamepadAxisAction {
    //    float              dead_zone{};
    //    input::GamepadAxis axis{};
    //};

    template <typename T>
    concept ActionT = (std::is_same_v<T, KeyboardAction>) ||
                      (std::is_same_v<T, MouseAction>) ||
                      (std::is_same_v<T, JoystickAction>) ||
                      (std::is_same_v<T, GamepadButtonAction>) ||
                      (std::is_same_v<T, GamepadAxisAction>);

    class FORR_API InputSystem {
    public:
        template <ActionT ActionKind>
        struct StoredAction {
            fe::string_hash action_hashed_name{};
            ActionKind      action_data{};
        };

    public:
        InputSystem(EventBus& event_bus);
        ~InputSystem() = default;

        // @brief add an action
        // 
        // @tparam ActionKind KeyboardAction, MouseAction, JoystickAction, GamepadButtonAction or GamepadAxisAction
        // 
        // @param action_name action name you're going to see in the GUI and use in 'fe::InputSystem::IsActionPressed()'-like functions
        // @param action_data the action will be marked as pressed, if the event bus will give an event with the same values as in action_data
        // 
        // @returns 'false' if the action name is already taken or ActionKind is unknown and 'true' if the action created successfully
        template <ActionT ActionKind>
        bool AddAction(fe::hashed_string_view action_name, const ActionKind& action_data) {
            if (this->IsActionRegistered(action_name)) return false;

            if constexpr (std::is_same_v<ActionKind, fe::KeyboardAction>) {
                m_KeyboardActions.push_back({ action_name, action_data });
            }
            else if constexpr (std::is_same_v<ActionKind, fe::MouseAction>) {
                m_MouseActions.push_back({ action_name, action_data });
            }
            //else if constexpr (std::is_same_v<ActionKind, fe::JoystickAction>) {
            //    m_JoystickActions.push_back({ action_name, action_data });
            //}
            //else if constexpr (std::is_same_v<ActionKind, fe::GamepadButtonAction>) {
            //    m_GamepadButtonActions.push_back({ action_name, action_data });
            //}
            //else if constexpr (std::is_same_v<ActionKind, fe::GamepadAxisAction>) {
            //    m_GamepadAxisActions.push_back({ action_name, action_data });
            //}
            else {
                static_assert(std::false_type::value && "Unknown action kind");
                return false;
            }

            m_ActionStates.insert({ action_name.get_hash(), {} });

            return true;
        }

        // @brief check if this name is already taken
        FORR_NODISCARD bool IsActionRegistered(fe::string_hash action_hashed_name) const {
            const auto it = m_ActionStates.find(action_hashed_name);
            return it != m_ActionStates.end();
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

        // @brief is action pressed or repeated
        FORR_NODISCARD bool IsActionPressed(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.is_pressed : false;
        }

        // @brief is action pressed in this current frame
        FORR_NODISCARD bool IsActionJustPressed(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.just_pressed : false;
        }

        // @brief is action released in this current frame
        FORR_NODISCARD bool IsActionJustReleased(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.just_released : false;
        }

        // @brief action value from '0.0f' to '1.0f' ( just '1.0f' for buttons )
        FORR_NODISCARD float GetActionValue(fe::string_hash action) const {
            auto it = m_ActionStates.find(action);
            return it != m_ActionStates.end() ? it->second.value : 0.0f;
        }

    private:
        void keyButtonCallback(const event::KeyButton& key_button);
        void unicodeCharTypedCallback(const event::UnicodeCharTyped& unicode_char_typed);
        void mouseButtonCallback(const event::MouseButton& mouse_button);

    private:
        std::reference_wrapper<EventBus> m_EventBus;

        // hashed action name --> is action active
        std::unordered_map<fe::string_hash, ActionState> m_ActionStates{};

        std::vector<StoredAction<KeyboardAction>>      m_KeyboardActions{};
        std::vector<StoredAction<MouseAction>>         m_MouseActions{};
        //std::vector<StoredAction<JoystickAction>>      m_JoystickActions{};
        //std::vector<StoredAction<GamepadButtonAction>> m_GamepadButtonActions{};
        //std::vector<StoredAction<GamepadAxisAction>>   m_GamepadAxisActions{};
    };
} // namespace fe
