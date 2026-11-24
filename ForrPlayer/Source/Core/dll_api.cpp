/*===============================================

    Forr Engine

    File : dll_api.cpp
    Role : provides an interface to register a dll module ( a game or a mod )
           and unified function to load, unload and get symbol addresses from the loaded dll

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#include "Forr/PCH/pch.hpp"
#include "Forr/Core/dll_api.hpp"

#ifdef _WIN32
#include <Windows.h>
constexpr std::string_view LIB_EXT = ".dll";
#elifdef __APPLE__
#include <dlfcn.h>
constexpr std::string_view LIB_EXT = ".dylib";
#else
#include <dlfcn.h>
constexpr std::string_view LIB_EXT = ".so";
#endif

std::expected<fe::dll_api::library_handle_raw, fe::dll_api::library_loading_error>
fe::dll_api::load_library_raw(const std::filesystem::path& full_path) {
    if (full_path.empty()) {
        return std::unexpected{ library_loading_error::PATH_WAS_EMPTY };
    }

    std::filesystem::path this_path{ full_path };

    if (!this_path.has_extension()) {
        this_path += LIB_EXT;
    }
    else if (this_path.extension() != LIB_EXT) {
        return std::unexpected{ library_loading_error::WRONG_PATH_EXTENSION };
    }

#ifdef _WIN32
    void* result = reinterpret_cast<void*>(LoadLibrary(this_path.wstring().c_str()));
#else
    void* result = dlopen(this_path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif

    if (!result) {
        return std::unexpected{ library_loading_error::FAILED_TO_LOAD_LIBRARY };
    }

    fe::dll_api::set_current_library(result);

    return library_handle_raw{ result };
}

std::expected<fe::dll_api::symbol_handle_raw, fe::dll_api::symbol_getting_error>
fe::dll_api::get_symbol_address_raw(fe::dll_api::library_handle_raw loaded_library_handle, std::string_view symbol_name) {
    if (!loaded_library_handle) {
        return std::unexpected{ symbol_getting_error::LIBRARY_HANDLE_WAS_NULL };
    }

    if (symbol_name.empty()) {
        return std::unexpected{ symbol_getting_error::SYMBOL_NAME_WAS_EMPTY };
    }

    std::string name_str{ symbol_name };

#ifdef _WIN32
    FARPROC raw_symbol = GetProcAddress(reinterpret_cast<HMODULE>(loaded_library_handle), name_str.c_str());
#else
    void* raw_symbol = dlsym(loaded_library_handle, name_str.c_str());
#endif

    if (!raw_symbol) {
        return std::unexpected{ symbol_getting_error::SYMBOL_NOT_FOUND };
    }

    return raw_symbol;
}

bool fe::dll_api::free_library_raw(fe::dll_api::library_handle_raw loaded_library_handle) {
    if (!loaded_library_handle) {
        return false;
    }

#ifdef _WIN32
    return FreeLibrary(reinterpret_cast<HMODULE>(loaded_library_handle)) != 0;
#else
    return dlclose(loaded_library_handle) == 0;
#endif
}

thread_local fe::dll_api::library_handle_raw G_CURRENT_LIBRARY = nullptr;

void fe::dll_api::set_current_library(fe::dll_api::library_handle_raw current_handle) noexcept {
    G_CURRENT_LIBRARY = current_handle;
}

fe::dll_api::library_handle_raw fe::dll_api::get_current_library() noexcept {
    return G_CURRENT_LIBRARY;
}
