/*===============================================

    Forr Engine

    File : InputManager.hpp
    Role : InputManager is a class to work with the game actions

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "InputTypes.hpp"
#include "Forr/Events/EventBus.hpp"

namespace fe {
    class InputManager {
    public:
        InputManager(EventBus& event_bus) : m_EventBus(event_bus) {}
        ~InputManager() = default;

        void AddAction() {

        }

    private:
        std::reference_wrapper<EventBus> m_EventBus;
    };
} // namespace fe
