/*===============================================

    Forr Engine

    File : InputSystem.cpp
    Role : InputSystem is a class to work with the game actions

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Input/InputSystem.hpp"

#include "Forr/Events/Events.hpp"

fe::InputSystem::InputSystem(EventBus& event_bus) : m_EventBus(event_bus) {
    //m_EventBus.get().Subscribe([&](const events::KeyButton& key_button) {
    //    for (auto& [name, action] : m_KeyboardActions) {
    //        action.key          = key_button.key;
    //        action.mods         = key_button.mods;
    //        action.scan_code    = key_button.scan_code;
    //        action.unicode_char = {};
    //    }
    //});

    //m_EventBus.get().Subscribe([&](const events::UnicodeCharTyped& unicode_char) {
    //    for (auto& [name, action] : m_KeyboardActions) {
    //        action.key          = {};
    //        action.mods         = {};
    //        action.scan_code    = {};
    //        action.unicode_char = unicode_char.code_point;
    //    }
    //});
}
