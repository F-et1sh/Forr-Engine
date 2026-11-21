/*===============================================

    Forr Engine

    File : dll_module_api.hpp
    Role : provides an interface to register a dll module ( a game or a mod )
           and unified function to load, unload and get process address from a dll

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <utility>
#include <filesystem>
#include <expected>

#include "attributes.hpp"
#include "context.hpp"
#include "string.hpp"

namespace fe::dll_api {
    struct component {
        using on_construct_func = void (*)(void*);
        using on_destruct_func  = void (*)(void*);

        string_raw  name{};
        string_hash type_hash{};
        size_t      size{};
        size_t      alignment{};

        on_construct_func on_construct{};
        on_destruct_func  on_destruct{};
    };

    struct engine_api {
        using register_component_func = void (*)(const component* desc);
        using register_system_func    = void (*)(const char* name, void (*update_func)(float delta_time));

        using add_component_func = void* (*) (uint32_t entity_id, string_hash type_hash);
        using get_component_func = void* (*) (uint32_t entity_id, string_hash type_hash);

        register_component_func register_component{};
        register_system_func    register_system{};

        add_component_func add_component{};
        get_component_func get_component{};
    };

    // game or mod
    struct application_module {

        using get_info_func = const char* (*) ();

        using on_initialize_func = void (*)(const engine_api*);
        using on_update_func     = void (*)(const engine_api*);
        using on_release_func    = void (*)(const engine_api*);

        get_info_func get_info{};

        string_raw name{};
        bool       is_primary{};

        on_initialize_func on_initialize{};
        on_update_func     on_update{};
        on_release_func    on_release{};
    };

    template <typename T>
    void register_component(const engine_api* api, const char* name) {
        component desc{};
        desc.name         = name;
        desc.type_hash    = fe::make_hash(name);
        desc.size         = sizeof(T);
        desc.alignment    = alignof(T);
        desc.on_construct = [](void* ptr) { new (ptr) T(); };
        desc.on_destruct  = [](void* ptr) { static_cast<T*>(ptr)->~T(); };
        api->register_component(&desc);
    }

    class library_handle; // forward declaration

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

    FORR_NODISCARD FORR_API std::expected<library_handle, library_loading_error> load_library(const std::filesystem::path& full_path);
    FORR_NODISCARD FORR_API std::expected<symbol_handle_raw, symbol_getting_error> get_symbol_address_raw(library_handle_raw loaded_library_handle, std::string_view symbol_name);
    // @returns 'false', if 'library_handle' was null or if something went wrong
    FORR_API bool free_library_raw(library_handle_raw loaded_library_handle);

    class library_handle {
        struct deleter {
            void operator()(void* ptr) const noexcept {
                if (ptr) fe::dll_api::free_library_raw(ptr);
            }
        };

    public:
        library_handle() = default;
        explicit library_handle(void* handle) noexcept : m_handle(handle) {}

        template <typename FuncPtr>
            requires std::is_pointer_v<FuncPtr>
        FORR_NODISCARD std::expected<FuncPtr, symbol_getting_error> get_symbol_address(std::string_view symbol_name) const {
            auto result = fe::dll_api::get_symbol_address_raw(m_handle.get(), symbol_name);
            if (result.has_value()) {
                return reinterpret_cast<FuncPtr>(result.value());
            }
            return std::unexpected{ result.error() };
        }

        FORR_NODISCARD library_handle_raw get() const noexcept { return m_handle.get(); }
        explicit                          operator bool() const noexcept { return m_handle != nullptr; }

    private:
        std::unique_ptr<void, deleter> m_handle{ nullptr };
    };

    using create_application_module = application_module* (*) ();

#ifdef _WIN32
#define FORR_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define FORR_MODULE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

// this define creates a function to be found by the 'ForrGame' or 'ForrEditor' via 'fe::dll_api::load_library()'
// use it as an entry point into your game/mod
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
