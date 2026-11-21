/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrGame

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "Forr/Core/dll_module_api.hpp"

void on_initialize(const fe::dll_api::engine_api* engine_api) {
}

const char* get_info() {
    static std::string message{ "ForrGame-Example executed" };
    return message.c_str();
}

extern "C" __declspec(dllexport) fe::dll_api::application_module* get_application_module() {
    static fe::dll_api::application_module result{
        &get_info,
        "ForrGame-Example",
        true,
        &on_initialize,
        nullptr,
        nullptr,
    };
    return &result;
}
