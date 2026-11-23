/*===============================================

    Forr Engine

    File : Systems.hpp
    Role : provides helpers to work with ECS and EnTT itself

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include "Forr/Core/attributes.hpp"
#include "Forr/Core/string.hpp"

#include <entt/entt.hpp>

namespace fe {
    template <typename T>
    using SystemDataT = T::Data;

    struct EmptySystemDataT {};

    template <typename T>
    concept SystemConcept = requires(/*fe::EngineContext& context, */ SystemDataT<T>& system_data) {
        { T::Initialize(/*context, */ system_data) } -> std::same_as<void>;
        { T::Update(/*context, */ system_data) } -> std::same_as<void>;
        { T::Release(/*context, */ system_data) } -> std::same_as<void>;
    };

    struct System {
        using InitializeFunction = void (*)(/*fe::EngineContext& context, */ void*);
        using UpdateFunction     = void (*)(/*fe::EngineContext& context, */ void*);
        using ReleaseFunction    = void (*)(/*fe::EngineContext& context, */ void*);

        fe::fixed_string<32> name{};
        void*                mapped_data{};

        InitializeFunction initialize_function{};
        UpdateFunction     update_function{};
        ReleaseFunction    release_function{};

        System()  = default;
        ~System() = default;

        FORR_CLASS_MOVABLE(System)
        FORR_CLASS_NONCOPYABLE(System)
    };

    // this is the structure you get in 'fe::Scene::AddSystem<>' to
    //  access your mapped data and system's index in the array
    template <SystemConcept System>
    struct SystemHandle {
        using SystemDataT = SystemDataT<System>::Data;

        SystemDataT* mapped_data{};
        // system's index in 'fe::Scene::m_Systems'
        uint32_t               system_index{};
        fe::hashed_string_view system_name{};

        SystemHandle()  = default;
        ~SystemHandle() = default;

        FORR_CLASS_MOVABLE(SystemHandle)
        FORR_CLASS_NONCOPYABLE(SystemHandle)
    };

    class Scene {
    public:
        Scene()  = default;
        ~Scene() = default;

        FORR_CLASS_MOVABLEONLY(Scene)

        template <SystemConcept System>
        FORR_NODISCARD SystemHandle<System> AddSystem(fe::fixed_string_view<32> name) {
            using SystemDataT = SystemDataT<System>::Data;

            auto& system = m_Systems.emplace_back();
            system.name  = name;

            SystemHandle<System> system_handle{};
            system_handle.render_pass_index = m_Systems.size() - 1;
            system_handle.render_pass_name  = name;

            if constexpr (!std::is_same_v<SystemDataT, EmptySystemDataT>) {
                std::byte*   mapped_data_raw = m_SystemsData.allocate(sizeof(std::decay_t<SystemDataT>), alignof(SystemDataT));
                SystemDataT* mapped_data     = new (mapped_data_raw) SystemDataT();

                system.mapped_data        = mapped_data;
                system_handle.mapped_data = mapped_data;

                system.initialize_function = [](/*fe::EngineContext& context, */ void* system_data) { System::Initialize(/*fe::EngineContext& context, */ *reinterpret_cast<SystemDataT*>(system_data)); };
                system.update_function     = [](/*fe::EngineContext& context, */ void* system_data) { System::Update(/*fe::EngineContext& context, */ *reinterpret_cast<SystemDataT*>(system_data)); };
                system.release_function    = [](/*fe::EngineContext& context, */ void* system_data) {
                    System::Release(/*fe::EngineContext& context, */ *reinterpret_cast<SystemDataT*>(system_data));
                    std::destroy_at(reinterpret_cast<SystemDataT*>(system_data));
                };
            }
            else {
                system.mapped_data        = nullptr;
                system_handle.mapped_data = nullptr;

                system.initialize_function = [](/*fe::EngineContext& context, */ void* system_data) { System::Initialize(/*fe::EngineContext& context, */); };
                system.update_function     = [](/*fe::EngineContext& context, */ void* system_data) { System::Update(/*fe::EngineContext& context, */); };
                system.release_function    = [](/*fe::EngineContext& context, */ void* system_data) { System::Release(/*fe::EngineContext& context, */); };
            }

            return system_handle;
        }

    private:
        std::vector<System> m_Systems{};
        fe::Arena           m_SystemsData{ 16 * 1024 };
    };

    enum class SystemStage : uint8_t {
        PRE_UPDATE,
        UPDATE,
        POST_UPDATE,
        RENDER,
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
