/*===============================================

    Forr Engine

    File : InputSystem.cpp
    Role : InputSystem is a class to work with the game actions

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Input/InputSystem.hpp"

fe::InputSystem::InputSystem(EventBus& event_bus) : m_EventBus(event_bus) {
    m_EventBus.get().Subscribe<&InputSystem::keyboardCallback>(*this);
}

void fe::InputSystem::keyboardCallback(const events::KeyButton& key_button) {

}
