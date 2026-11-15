/*===============================================

    Forr Engine

    File : dll_module_api.hpp
    Role : provides an interface to register your dll module ( a game or a mod )

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "context.hpp"
#include "string.hpp"

namespace fe {
    struct component_desc {
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
        using register_component_func = void (*)(const component_desc* desc);
        using register_system_func    = void (*)(const char* name, void (*update_func)(float delta_time));

        using add_component_func = void* (*) (uint32_t entity_id, string_hash type_hash);
        using get_component_func = void* (*) (uint32_t entity_id, string_hash type_hash);

        register_component_func register_component{};
        register_system_func    register_system{};

        add_component_func add_component{};
        get_component_func get_component{};
    };

    struct module_desc {
        using on_initialize_func = void (*)(const engine_api* context);
        using on_update_func     = void (*)(const engine_api* context);
        using on_release_func    = void (*)(const engine_api* context);

        string_raw name{};
        bool       is_primary{};

        on_initialize_func on_initialize{};
        on_update_func     on_update{};
        on_release_func    on_release{};
    };

    using get_module_func = const module_desc* (*) ();

    template <typename T>
    void register_component(const engine_api* api, const char* name) {
        component_desc desc{};
        desc.name         = name;
        desc.type_hash    = fe::make_hash(name);
        desc.size         = sizeof(T);
        desc.alignment    = alignof(T);
        desc.on_construct = [](void* ptr) { new (ptr) T(); };
        desc.on_destruct  = [](void* ptr) { static_cast<T*>(ptr)->~T(); };
        api->register_component(&desc);
    }

    // this define creates a function to be found by the 'ForrGame' or 'ForrEditor' via 'LoadLibrary()'
    // use it as an entry point into your game/mod
#define FORR_REGISTER_DLL_MODULE(name, is_primary, on_initialize, on_update, on_release) \
    extern "C" __declspec(dllexport) const ModuleDesc* GetModule() {                     \
        static module_desc result{                                                       \
            name,                                                                        \
            is_primary,                                                                  \
            on_initialize,                                                               \
            on_update,                                                                   \
            on_release,                                                                  \
        };                                                                               \
        return &result;                                                                  \
    }

} // namespace fe
