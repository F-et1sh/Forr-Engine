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

FORR_REGISTER_DLL_MODULE("ForrGame-Example", true, &on_initialize, nullptr, nullptr)