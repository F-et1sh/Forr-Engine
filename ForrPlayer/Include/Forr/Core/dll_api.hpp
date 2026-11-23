/*===============================================

    Forr Engine

    File : dll_api.hpp
    Role : provides an interface to register a dll module ( a game or a mod )
           and unified function to load, unload and get symbol addresses from the loaded dll

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <utility>
#include <filesystem>
#include <expected>

#include "attributes.hpp"
#include "context.hpp"
#include "string.hpp"

namespace fe::dll_api {
    struct engine_api {
        void* instance{};

        const char* (*get_message)(void* instance);
    };

    // game or mod
    struct application_module {
        using on_initialize_func = void (*)(const engine_api*);
        using on_update_func     = void (*)(const engine_api*);
        using on_release_func    = void (*)(const engine_api*);

        using return_message_func = const char* (*)();

        string_raw name{};
        bool       is_primary{};

        on_initialize_func on_initialize{};
        on_update_func     on_update{};
        on_release_func    on_release{};

        return_message_func return_message{};
    };

    using library_handle_raw = void*;
    using symbol_handle_raw  = void*;

    enum class library_loading_error : uint8_t {
        PATH_WAS_EMPTY,
        WRONG_PATH_EXTENSION,
        FAILED_TO_LOAD_LIBRARY
    };

    enum class symbol_getting_error : uint8_t {
        LIBRARY_HANDLE_WAS_NULL,
        SYMBOL_NAME_WAS_EMPTY,
        SYMBOL_NOT_FOUND
    };

    FORR_NODISCARD FORR_API std::expected<library_handle_raw, library_loading_error> load_library_raw(const std::filesystem::path& full_path);
    FORR_NODISCARD FORR_API std::expected<symbol_handle_raw, symbol_getting_error> get_symbol_address_raw(library_handle_raw loaded_library_handle, std::string_view symbol_name);
    // @returns 'false', if 'library_handle_raw' was null or if something went wrong
    FORR_API bool free_library_raw(library_handle_raw loaded_library_handle);

    struct deleter {
        void operator()(void* ptr) const noexcept {
            if (ptr) fe::dll_api::free_library_raw(ptr);
        }
    };
    using library_handle = std::unique_ptr<void, deleter>;

#ifdef _WIN32
#define FORR_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define FORR_MODULE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// this define creates a function to be found by the 'ForrRuntime' or 'ForrEditor' via 'fe::dll_api::load_library()'
// use it as an entry point into your game/mod
//
// @param name name of the dll module
// @param is_primary defines is this a game or a mod for the game. primary dll modules are loaded first
// @param on_initialize a pointer to a function, which will be called on initialize. you can put 'nullptr' here
// @param on_update a pointer to a function, which will be called every frame. you can put 'nullptr' here
// @param on_release a pointer to a function, which will be called on release. you can put 'nullptr' here
//
// @returns the built application desc, which is going to be used by the executer : ForrRuntime or ForrEditor
#define FORR_REGISTER_DLL_MODULE(name, is_primary, on_initialize, on_update, on_release) \
    FORR_MODULE_EXPORT fe::dll_api::application_module* get_application_module() {       \
        static fe::dll_api::application_module result{                                   \
            name,                                                                        \
            is_primary,                                                                  \
            on_initialize,                                                               \
            on_update,                                                                   \
            on_release,                                                                  \
        };                                                                               \
        return &result;                                                                  \
    }

} // namespace fe::dll_api
