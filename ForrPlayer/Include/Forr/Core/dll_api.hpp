/*===============================================

    Forr Engine

    File : dll_api.hpp
    Role : provides an interface to register a dll module ( a game or a mod )
           and unified function to load, unload and get symbol addresses from the loaded dll

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <memory>
#include <utility>
#include <filesystem>
#include <expected>
#include <functional>
#include <vector>
#include <ranges>
#include <type_traits>

#include "attributes.hpp"
#include "context.hpp"
#include "string.hpp"
#include "unique_handle.hpp"

namespace fe::dll_api {
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

    // @brief RAII class to track loaded dll library's interactions with the engine;
    // adding a render pass to the render graph, for example. this class guarantees that 'fe::RenderGraph::RemovePass()'
    // will called before the library unloaded
    class module_context {
    public:
        module_context() = default;
        module_context(library_handle_raw library_handle_raw) : m_library_handle(library_handle_raw) {}
        module_context(library_handle loaded_library_handle) : m_library_handle(std::move(loaded_library_handle)) {}
        explicit module_context(const std::filesystem::path& full_path) {
            auto result = load_library_raw(full_path);
            if (!result.has_value()) {
                fe::logging::error("Failed to initialize a module context. Failed to load the library\nPath : %s", full_path.generic_u8string().c_str());
                return;
            }
            m_library_handle = library_handle{ result.value() };
        }
        ~module_context() {
            this->unload();
        }

        FORR_CLASS_MOVABLEONLY(module_context)

        // @brief takes ownership of a move-only RAII handle and tracks its lifetime;
        // this function moves your scoped handle ( created via 'fe::create_unique_handle' ) and tracks its lifetime;
        // you can use it to not create a field for every scoped handle you have to track
        //
        // @tparam ScopedHandleT a move-only handle type, usually created via 'fe::create_unique_handle'
        //
        // @param scoped_handle the scoped handle to take ownership of
        //
        // @returns lvalue reference of the stored handle
        template <typename ScopedHandleT>
            requires(!std::is_lvalue_reference_v<ScopedHandleT>)
        ScopedHandleT& own_scoped(ScopedHandleT&& scoped_handle) {
            auto           ptr = std::make_unique<ScopedHandleT>(std::forward<ScopedHandleT>(scoped_handle));
            ScopedHandleT& ref = *ptr;
            m_cleanup.emplace_back([p = std::move(ptr)]() {});
            return ref;
        }

        // @brief add a lambda that will be called in the destructor
        template <typename Fn>
        void on_unload(Fn&& fn) {
            m_cleanup.emplace_back(std::forward<Fn>(fn));
        }

        // @brief call all lambdas added via 'module_context::on_unload()';
        // this function runs them in reverse order
        void unload() {
            for (auto& fn : std::views::reverse(m_cleanup)) {
                fn();
            }
            m_cleanup.clear();
        }

        // @brief get raw loaded dll library's handle
        //
        // @returns raw library handle
        library_handle_raw get_library_handle_raw() const noexcept {
            return m_library_handle.get();
        }

    private:
        std::vector<std::move_only_function<void()>> m_cleanup{};
        library_handle                               m_library_handle{};
    };

    // game or mod
    struct application_desc {
        using on_initialize_func = void (*)(context&, module_context&);
        using on_update_func     = void (*)(context&, module_context&);
        using on_release_func    = void (*)(context&, module_context&);

        string_raw name{};
        bool       is_primary{};

        on_initialize_func on_initialize{};
        on_update_func     on_update{};
        on_release_func    on_release{};
    };

    using create_application_desc = application_desc* (*) ();

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
    FORR_MODULE_EXPORT fe::dll_api::application_desc* get_application_desc() {           \
        static fe::dll_api::application_desc result{                                     \
            name,                                                                        \
            is_primary,                                                                  \
            on_initialize,                                                               \
            on_update,                                                                   \
            on_release,                                                                  \
        };                                                                               \
        return &result;                                                                  \
    }

} // namespace fe::dll_api
