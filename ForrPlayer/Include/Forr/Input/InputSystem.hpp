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
            //float dead_zone{ 0.5f }; // how many points needed to trigger this action

            input::KeyCode  key{};
            input::Action   action{};
            uint8_t         mods{};
            input::ScanCode scan_code{};
        };

    public:
        InputSystem(EventBus& event_bus);
        ~InputSystem() = default;

        void AddAction() {
        }

    private:
        std::reference_wrapper<EventBus> m_EventBus;

        // action name --> action
        std::unordered_map<fe::hashed_wstring, KeyboardAction> m_KeyboardActions{};
    };
} // namespace fe
