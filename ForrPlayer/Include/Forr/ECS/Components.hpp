/*===============================================

    Forr Engine

    File : Components.hpp
    Role : ECS ( Entity System ) components.
        All structures here must be POD - this ECS is data-oriented.

        TODO : think about adding that "DOD fe::component::Transform --> OOP fe::TransformComponent" like in Resource Management system

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <utility>
#include <variant>

#include "Forr/Core/pointer.hpp"
#include "Forr/ResourceManagement/Resources.hpp"

#include <entt/entt.hpp>

namespace fe {
    using ReactiveDirtyStorage = entt::reactive_mixin<entt::storage<void>>;

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

namespace fe::component {
    struct Transform {
        glm::vec3 position{};
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };

    struct LocalToWorld {
        glm::mat4 matrix{};
    };

    struct Camera {
        struct PerspectiveSettings {
            float fov{};
            float aspect_ratio{};

            float near_plane{};
            float far_plane{};
        };

        struct OrthographicSettings {
            float left{};
            float right{};
            float bottom{};
            float top{};

            float near_plane{};
            float far_plane{};
        };

        std::variant<PerspectiveSettings,
                     OrthographicSettings>
             camera_settings{};
        bool is_primary{};
    };

    struct ViewProjection {
        glm::mat4 projection{};
        glm::mat4 view{};
        glm::mat4 view_projection{};
    };

    struct Light {
        glm::mat4 transform{}; // temp

        Light(const glm::mat4& transform)
            : transform(transform) {}

        Light() = default;
    };

    struct Model {
        fe::pointer<resource::Model> model_ptr{};

        Model(fe::pointer<resource::Model> model_ptr) : model_ptr(model_ptr) {}
        Model() = default;
    };
} // namespace fe::component
