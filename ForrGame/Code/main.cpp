/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrGame

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/Core/dll_api.hpp"

void on_initialize(const fe::dll_api::engine_api* engine_api) {
    
}

void on_update(const fe::dll_api::engine_api* engine_api) {
    
}

void on_release(const fe::dll_api::engine_api* engine_api) {
    
}

extern "C" __declspec(dllexport) fe::dll_api::application_module* get_application_module() {
    static fe::dll_api::application_module result{
        "ForrGame-Example",
        true,
        &on_initialize,
        &on_update,
        &on_release
    };
    return &result;
}