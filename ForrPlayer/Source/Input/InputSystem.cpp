/*===============================================

    Forr Engine

    File : InputSystem.cpp
    Role : InputSystem is a class to work with the game actions

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Input/InputSystem.hpp"

#include <utility>

fe::InputSystem::InputSystem(EventBus& event_bus) : m_EventBus(event_bus) {
    m_EventBus.get().Subscribe<&InputSystem::keyButtonCallback>(*this);
    m_EventBus.get().Subscribe<&InputSystem::unicodeCharTypedCallback>(*this);
    m_EventBus.get().Subscribe<&InputSystem::mouseButtonCallback>(*this);
}

void fe::InputSystem::keyButtonCallback(const event::KeyButton& key_button) {
    for (auto& stored_action : m_KeyboardActions) {
        const auto& action_key_code = stored_action.action_data.code;
        const auto& action_key_mods = stored_action.action_data.mods;

        auto& action_state = m_ActionStates[stored_action.action_hashed_name];

        auto lambda = [&action_key_code, &action_key_mods, &action_state, &key_button](const auto& to_compare) -> void {
            using Code = std::decay_t<decltype(to_compare)>;

            if (!std::holds_alternative<Code>(action_key_code)) {
                return;
            }

            if (std::get<Code>(action_key_code) != to_compare) {
                return;
            }

            bool is_pressed = (key_button.action == input::Action::PRESS ||
                               key_button.action == input::Action::REPEAT) &&
                              action_key_mods == key_button.mods;

            action_state.just_pressed  = is_pressed && !action_state.is_pressed;
            action_state.just_released = !is_pressed && action_state.is_pressed;

            action_state.is_pressed = is_pressed;
            action_state.value      = static_cast<float>(action_state.is_pressed);
        };

        lambda(key_button.key);
        lambda(key_button.scan_code);
    }
}

void fe::InputSystem::unicodeCharTypedCallback(const event::UnicodeCharTyped& unicode_char_typed) {
    for (auto& stored_action : m_KeyboardActions) {
        const auto& action_key_code = stored_action.action_data.code;
        const auto& action_key_mods = stored_action.action_data.mods;

        auto& action_state = m_ActionStates[stored_action.action_hashed_name];

        if (std::holds_alternative<input::UnicodeChar>(action_key_code)) {
            bool is_pressed = (std::get<input::UnicodeChar>(action_key_code) == unicode_char_typed.code_point) &&
                              action_key_mods == unicode_char_typed.mods;

            action_state.just_pressed  = is_pressed && !action_state.is_pressed;
            action_state.just_released = !is_pressed && action_state.is_pressed;

            action_state.is_pressed = is_pressed;
        }
    }
}

void fe::InputSystem::mouseButtonCallback(const event::MouseButton& mouse_button) {
    for (auto& stored_action : m_MouseActions) {
        const auto& action_button = stored_action.action_data.button;
        const auto& action_mods   = stored_action.action_data.mods;

        auto& action_state = m_ActionStates[stored_action.action_hashed_name];

        bool is_pressed = (action_button == mouse_button.button) &&
                          (mouse_button.action == input::Action::PRESS ||
                           mouse_button.action == input::Action::REPEAT) &&
                          action_mods == mouse_button.mods;

        action_state.just_pressed  = is_pressed && !action_state.is_pressed;
        action_state.just_released = !is_pressed && action_state.is_pressed;

        action_state.is_pressed = is_pressed;
    }
}
