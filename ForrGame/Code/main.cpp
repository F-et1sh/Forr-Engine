/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrGame

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/Core/dll_api.hpp"

static std::string message{};

void on_initialize(const fe::dll_api::engine_api* engine_api) {
    message = engine_api->get_message(engine_api->instance);
}

const char* return_message() {
    static std::string message2 = message + " -- 2";
    return message2.c_str();
}

extern "C" __declspec(dllexport) fe::dll_api::application_module* get_application_module() {
    static fe::dll_api::application_module result{
        "ForrGame-Example",
        true,
        &on_initialize,
        nullptr,
        nullptr,
        &return_message
    };
    return &result;
}