/*===============================================

    Forr Engine

    File : Scene.hpp
    Role : provides helpers to work with ECS and EnTT itself

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once

#include <ranges>
#include <algorithm>

#include "Forr/Core/attributes.hpp"
#include "Forr/Core/string.hpp"
#include "Forr/Core/pointer.hpp"
#include "Forr/Core/context.hpp"

#ifdef FORRENGINE_EXPORTS
    #define ENTT_API_EXPORT
#else
    #define ENTT_API_IMPORT
#endif

#include <entt/entt.hpp>

namespace fe {
    struct EmptySystemDataT {};

    template <typename Context, typename... Args>
    struct ExtractSystemDataTypeTraits {
        using SystemDataT = std::tuple_element_t<1, std::tuple<Args...>>;
    };

    template <typename Context, typename FirstArg>
    struct ExtractSystemDataTypeTraits<Context, FirstArg> {
        using SystemDataT = EmptySystemDataT;
    };

    template <typename T>
    struct SystemFnTraits;

    template <typename ReturnType, typename... Args>
    struct SystemFnTraits<ReturnType (*)(Args...)> {
        using SystemDataT_Initialize = ExtractSystemDataTypeTraits<fe::context, std::decay_t<Args>...>::SystemDataT;
        using SystemDataT_Update     = ExtractSystemDataTypeTraits<fe::context, std::decay_t<Args>...>::SystemDataT;
        using SystemDataT_Release    = ExtractSystemDataTypeTraits<fe::context, std::decay_t<Args>...>::SystemDataT;

        static_assert((sizeof...(Args) == 1) || (sizeof...(Args) == 2), "Wrong number of arguments");
    };

    template <typename SystemT>
    struct SystemTraits {
    private:
        using SystemDataT_Initialize = SystemFnTraits<decltype(&SystemT::Initialize)>::SystemDataT_Initialize;
        using SystemDataT_Update     = SystemFnTraits<decltype(&SystemT::Update)>::SystemDataT_Update;
        using SystemDataT_Release    = SystemFnTraits<decltype(&SystemT::Release)>::SystemDataT_Release;

        static_assert(std::is_same_v<SystemDataT_Initialize, SystemDataT_Update> &&
                          std::is_same_v<SystemDataT_Update, SystemDataT_Release>,
                      "SystemT::Initialize(), SystemT::Update() and SystemT::Release() accept different data types");

    public:
        using SystemDataT = SystemDataT_Initialize;
    };

    template <typename T>
    concept SystemConcept = requires(fe::context& context, SystemTraits<T>::SystemDataT& system_data) {
        { T::Initialize(context, system_data) } -> std::same_as<void>;
        { T::Update(context, system_data) } -> std::same_as<void>;
        { T::Release(system_data) } -> std::same_as<void>;
    } || requires(fe::context& context) {
        { T::Initialize(context) } -> std::same_as<void>;
        { T::Update(context) } -> std::same_as<void>;
        { T::Release() } -> std::same_as<void>;
    };

    struct System {
        enum class Stage : uint8_t {
            PRE_UPDATE,
            UPDATE,
            POST_UPDATE,
            RENDER
        };

        using InitializeFunction = void (*)(fe::context& context, void*);
        using UpdateFunction     = void (*)(fe::context& context, void*);
        using ReleaseFunction    = void (*)(void*);

        fe::fixed_string<32> name{};
        Stage                stage{};

        void* mapped_data{};

        InitializeFunction initialize_function{};
        UpdateFunction     update_function{};
        ReleaseFunction    release_function{};

        System() = default;
        ~System() {
            if (release_function && mapped_data) {
                release_function(mapped_data);
            }
        }

        FORR_CLASS_MOVABLEONLY(System)
    };

    // this is the structure you get in 'fe::Scene::AddSystem<>' to
    //  access your mapped data and to remove the system if needed
    template <SystemConcept SystemT>
    struct SystemHandle {
        using SystemDataT = SystemTraits<SystemT>::SystemDataT;

        SystemDataT*           mapped_data{};
        fe::pointer<System>    system_ptr{};
        fe::hashed_string_view system_name{};
    };

    class Scene {
    public:
        Scene()  = default;
        ~Scene() = default;

        FORR_CLASS_MOVABLEONLY(Scene)

        template <SystemConcept SystemT>
        FORR_NODISCARD SystemHandle<SystemT> AddSystem(fe::fixed_string_view<32> name, System::Stage stage = System::Stage::UPDATE) {
            using SystemDataT = SystemTraits<SystemT>::SystemDataT;

            System system{};
            system.name  = name;
            system.stage = stage;

            SystemHandle<SystemT> system_handle{};
            system_handle.system_name = name;

            if constexpr (!std::is_same_v<SystemDataT, EmptySystemDataT>) {
                std::byte*   mapped_data_raw = m_SystemsData.allocate(sizeof(std::decay_t<SystemDataT>), alignof(SystemDataT));
                SystemDataT* mapped_data     = new (mapped_data_raw) SystemDataT();

                system.mapped_data        = mapped_data;
                system_handle.mapped_data = mapped_data;

                system.initialize_function = [](fe::context& context, void* system_data) { SystemT::Initialize(context, *reinterpret_cast<SystemDataT*>(system_data)); };
                system.update_function     = [](fe::context& context, void* system_data) { SystemT::Update(context, *reinterpret_cast<SystemDataT*>(system_data)); };
                system.release_function    = [](void* system_data) {
                    SystemT::Release(*reinterpret_cast<SystemDataT*>(system_data));
                    std::destroy_at(reinterpret_cast<SystemDataT*>(system_data));
                };
            }
            else {
                system.mapped_data        = nullptr;
                system_handle.mapped_data = nullptr;

                system.initialize_function = [](fe::context& context, void* system_data) { SystemT::Initialize(context); };
                system.update_function     = [](fe::context& context, void* system_data) { SystemT::Update(context); };
                system.release_function    = [](void* system_data) { SystemT::Release(); };
            }

            system_handle.system_ptr = m_Systems.create(std::move(system));

            return system_handle;
        }

        template <SystemConcept SystemT>
        void RemoveSystem(SystemHandle<SystemT> system_handle) {
            m_Systems.destroy(system_handle.system_ptr);
        }

        // TODO : provide ability to invoke something between the stages
        void InitializeSystems(fe::context& context) {
            auto systems = this->getSorted();

            for (size_t i = 0; i < systems.size(); i++) {
                const auto& system = *m_Systems.get(systems[i]);
                system.initialize_function(context, system.mapped_data);
            }
        }

        // TODO : provide ability to invoke something between the stages
        void UpdateSystems(fe::context& context) {
            auto systems = this->getSorted();

            for (size_t i = 0; i < systems.size(); i++) {
                const auto& system = *m_Systems.get(systems[i]);
                system.update_function(context, system.mapped_data);
            }
        }

        // TODO : provide ability to invoke something between the stages
        void ReleaseSystems(fe::context& context) {
            auto systems = this->getSorted();

            for (size_t i = 0; i < systems.size(); i++) {
                const auto& system = *m_Systems.get(systems[i]);
                system.release_function(system.mapped_data);
            }
        }

    private:
        std::vector<fe::pointer<System>> getSorted() const {
            std::vector<fe::pointer<System>> order{};
            order.reserve(m_Systems.size());

            m_Systems.for_each([&order](fe::pointer<System> ptr) {
                order.push_back(ptr);
            });

            std::ranges::sort(order, [&](fe::pointer<System> a, fe::pointer<System> b) -> bool {
                return m_Systems.get(a)->stage < m_Systems.get(b)->stage;
            });

            return order;
        }

    private:
        fe::typed_pointer_storage<System> m_Systems{};
        fe::Arena                         m_SystemsData{ 16 * 1024 };
    };

    using ReactiveDirtyStorage = entt::reactive_mixin<entt::storage<void>>;

    // @brief a wrapper over 'entt::registry' that does not allow you to change the
    //        components without telling the 'entt::registry' about it
    class ReactiveRegistry {
    public:
        ReactiveRegistry(entt::registry& registry) : m_registry(registry) {}

        template <typename T, typename Func>
        decltype(auto) patch(entt::entity entity, Func&& func) {
            using return_type = decltype(m_registry.get().patch<T>(entity, std::forward<Func>(func)));

            if constexpr (std::is_void_v<return_type>) {
                m_registry.get().patch<T>(entity, std::forward<Func>(func));
            }
            else {
                return std::as_const(m_registry.get().patch<T>(entity, std::forward<Func>(func)));
            }
        }

        template <typename T, typename... Args>
        decltype(auto) emplace_or_replace(entt::entity entity, Args&&... args) {
            using return_type = decltype(m_registry.get().emplace_or_replace<T>(entity, std::forward<Args>(args)...));

            if constexpr (std::is_void_v<return_type>) {
                m_registry.get().emplace_or_replace<T>(entity, std::forward<Args>(args)...);
            }
            else {
                return std::as_const(m_registry.get().emplace_or_replace<T>(entity, std::forward<Args>(args)...));
            }
        }

        template <typename T, typename... Args>
        decltype(auto) replace(entt::entity entity, Args&&... args) {
            using return_type = decltype(m_registry.get().replace<T>(entity, std::forward<Args>(args)...));

            if constexpr (std::is_void_v<return_type>) {
                m_registry.get().replace<T>(entity, std::forward<Args>(args)...);
            }
            else {
                return std::as_const(m_registry.get().replace<T>(entity, std::forward<Args>(args)...));
            }
        }

        template <typename T, typename... Args>
        decltype(auto) emplace(entt::entity entity, Args&&... args) {
            using return_type = decltype(m_registry.get().emplace<T>(entity, std::forward<Args>(args)...));

            if constexpr (std::is_void_v<return_type>) {
                m_registry.get().emplace<T>(entity, std::forward<Args>(args)...);
            }
            else {
                return std::as_const(m_registry.get().emplace<T>(entity, std::forward<Args>(args)...));
            }
        }

        // there is no remove() function because you can't remove component without accessing
        // the registry, so there is no need in that function here
        // the main idea of 'ReactiveRegistry' is to prevent changing components without telling the 'entt::registry' about that

        template <typename T>
        FORR_NODISCARD decltype(auto) get(entt::entity entity) const {
            using return_type = decltype(m_registry.get().get<T>(entity));

            if constexpr (std::is_void_v<return_type>) {
                m_registry.get().get<T>(entity);
            }
            else {
                return std::as_const(m_registry.get().get<T>(entity));
            }
        }

        template <typename... Components>
        FORR_NODISCARD auto view() const {
            return std::as_const(m_registry.get()).view<Components...>();
        }

        FORR_NODISCARD const entt::registry& registry() const { return m_registry; }
        ReactiveRegistry&                    change_registry(entt::registry& registry_to) {
            m_registry = registry_to;
            return *this;
        }

        // @brief returns the non-const 'entt::registry', which is unsafe,
        //        because you can change a component and forget to tell the 'entt::registry' about that;
        //        use this function only, if you're aware what you are doing
        //
        //        for example, if you need the 'entt::registry' to pass into 'emplace_on_construct_of()' or 'remove_on_destroy_of()'
        //        and you have only 'ReactiveRegistry' nearby
        //
        // @returns non-const 'entt::registry'
        FORR_NODISCARD entt::registry& registry_unsafe() { return m_registry; }

    private:
        std::reference_wrapper<entt::registry> m_registry;
    };

    // @brief adds the entity's index to this 'dirty_storage',
    //        if any of the components from the template has been created via 'emplace_or_replace()'
    //        or 'emplace()' from 'entt::registry' or 'fe::ReactiveRegistry'
    //
    // @tparam Components the components you want to bind the storage to
    //
    // @param dirty_storage the storage you want see the entities' indices
    template <typename... Components>
    static void bind_to_construct_of(ReactiveDirtyStorage& dirty_storage) noexcept {
        (dirty_storage.on_construct<Components>(), ...);
    }

    template <typename... Components>
    static void unbind_from_construct_of(ReactiveDirtyStorage& storage) noexcept {
        (storage.registry().on_construct<Components>().disconnect(&storage), ...);
    }

    // @brief adds the entity's index to this 'dirty_storage',
    //        if any of the components from the template has been changed via 'patch()',
    //        'emplace_or_replace()' or 'replace()' from 'entt::registry' or 'fe::ReactiveRegistry'
    //
    // @tparam Components the components you want to bind the storage to
    //
    // @param dirty_storage the storage you want see the entities' indices
    template <typename... Components>
    static void bind_to_update_of(ReactiveDirtyStorage& dirty_storage) noexcept {
        (dirty_storage.on_update<Components>(), ...);
    }

    template <typename... Components>
    static void unbind_from_update_of(ReactiveDirtyStorage& storage) noexcept {
        (storage.registry().on_update<Components>().disconnect(&storage), ...);
    }

    // @brief adds the entity's index to this 'dirty_storage',
    //        if any of the components from the template has been destroyed via 'remove()'
    //        ( or any function that's doing it ) from 'entt::registry'
    //
    // @tparam Components the components you want to bind the storage to
    //
    // @param dirty_storage the storage you want see the entities' indices
    template <typename... Components>
    static void bind_to_destroy_of(ReactiveDirtyStorage& dirty_storage) noexcept {
        (dirty_storage.on_destroy<Components>(), ...);
    }

    template <typename... Components>
    static void unbind_from_destroy_of(ReactiveDirtyStorage& storage) noexcept {
        (storage.registry().on_destroy<Components>().disconnect(&storage), ...);
    }

    // @brief emplaces or replaces the 'RelatedComponent', if the 'MainComponent' has been constructed
    //
    // @tparam RelatedComponent component, which is going to be emplaced or replaced, if the 'MainComponent' has constructed
    // @tparam MainComponent component, the 'RelatedComponent' related to
    //
    // @param registry the registry there is rule is going to work
    // @param enable if 'true', then the rule will work; if 'false', then it will be disabled; 'true' by default
    //               if you want to disable this, then call this function and put 'false' here
    template <typename RelatedComponent, typename MainComponent>
    static void emplace_on_construct_of(entt::registry& registry, bool enable = true) noexcept {
        if (enable) {
            registry.on_construct<MainComponent>().template connect<&entt::registry::emplace_or_replace<RelatedComponent>>();
        }
        else {
            registry.on_construct<MainComponent>().template disconnect<&entt::registry::emplace_or_replace<RelatedComponent>>();
        }
    }

    // @brief removes the 'RelatedComponent', if the 'MainComponent' has been destroyed
    //
    // @tparam RelatedComponent component, which is going to be removed, if the 'MainComponent' has destroyed
    // @tparam MainComponent component, the 'RelatedComponent' related to
    //
    // @param registry the registry there is rule is going to work
    // @param enable if 'true', then the rule will work; if 'false', then it will be disabled; 'true' by default
    //               if you want to disable this, then call this function and put 'false' here
    template <typename RelatedComponent, typename MainComponent>
    static void remove_on_destroy_of(entt::registry& registry, bool enable = true) noexcept {
        if (enable) {
            registry.on_destroy<MainComponent>().template connect<&entt::registry::remove<RelatedComponent>>();
        }
        else {
            registry.on_destroy<MainComponent>().template disconnect<&entt::registry::remove<RelatedComponent>>();
        }
    }

    // @brief 'RelatedComponent' starts emplacing to the entity as the 'MainComponent' has emplaced
    //                           and removing as the 'MainComponent' has removed;
    //                           also, the 'dirty_storage' is going to be filled with entities, which's
    //                           'MainComponent' or any component from 'Other' have been constructed or updated
    //
    // @tparam RelatedComponent component, which is going to be emplaced, if the 'MainComponent' has been emplaced
    //                          and destroyed, if the 'MainComponent' has been removed
    // @tparam MainComponent component, the 'RelatedComponent' related to
    // @tparam Other dirty_storage is filled by entities, which's 'MainComponent' or any of components from 'Other'
    //               has been constructed or updated. you can call this 'affected by components'
    //
    // @param registry the registry there is rule is going to work
    // @param dirty_storage the reactive storage to put the entities, which's components has constructed or updated
    template <typename RelatedComponent, typename MainComponent, typename... Other>
    static void bind_related_component(ReactiveDirtyStorage& dirty_storage) noexcept {
        fe::emplace_on_construct_of<RelatedComponent, MainComponent>(dirty_storage.registry(), true);
        fe::remove_on_destroy_of<RelatedComponent, MainComponent>(dirty_storage.registry(), true);

        fe::bind_to_construct_of<MainComponent>(dirty_storage);
        fe::bind_to_update_of<MainComponent>(dirty_storage);

        (fe::bind_to_construct_of<Other>(dirty_storage), ...);
        (fe::bind_to_update_of<Other>(dirty_storage), ...);
    }

    template <typename RelatedComponent, typename MainComponent, typename... Other>
    static void unbind_related_component(ReactiveDirtyStorage& dirty_storage) noexcept {
        fe::emplace_on_construct_of<RelatedComponent, MainComponent>(dirty_storage.registry(), false);
        fe::remove_on_destroy_of<RelatedComponent, MainComponent>(dirty_storage.registry(), false);

        fe::unbind_from_construct_of<MainComponent>(dirty_storage);
        fe::unbind_from_update_of<MainComponent>(dirty_storage);

        (fe::unbind_from_construct_of<Other>(dirty_storage), ...);
        (fe::unbind_from_update_of<Other>(dirty_storage), ...);
    }

} // namespace fe
