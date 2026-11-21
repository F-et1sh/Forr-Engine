/*===============================================

    Forr Engine

    File : main.hpp
    Role : the main file in ForrRuntime

        ForrRuntime is a thin wrapper to execute games made with Forr-Engine

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include <filesystem>
#include <span>

#include "Forr/Core/dll_module_api.hpp"
#include "Forr/Core/path.hpp"

int main(int argc, char* argv[]) {
    if (argc < 1 || argv == nullptr) {
        fe::logging::fatal("No arguments passed. Execute the application with the first argument as a full path to the game's dll");
        return 0;
    }

    std::filesystem::path dll_full_path{ std::span(argv, argc)[1] };
    
    fe::logging::info("ready");

    auto library_loading_result = fe::dll_api::load_library(dll_full_path);
    if (!library_loading_result.has_value()) {
        fe::logging::fatal("Failed to load the library : error code %i", static_cast<uint8_t>(library_loading_result.error())); // TODO : print the full error message, not only the code
        return 0;
    }

    fe::logging::info("the library is loaded");

    fe::dll_api::library_handle library_handle = std::move(library_loading_result.value());
    auto symbol_address_result = library_handle.get_symbol_address<fe::dll_api::create_application_module>("get_application_module");
    if (!symbol_address_result.has_value()) {
        fe::logging::fatal("Failed to load a symbol 'get_application_module()' : error code %i", static_cast<uint8_t>(symbol_address_result.error())); // TODO : print the full error message, not only the code
        return 0;
    }
    
    fe::logging::info("the symbol is loaded");

    auto symbol_address = symbol_address_result.value();

    fe::logging::info(symbol_address()->get_info());

    return 0;
}
