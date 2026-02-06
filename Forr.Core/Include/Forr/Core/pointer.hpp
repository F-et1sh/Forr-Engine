/*===============================================

    Forr Engine - Core Module

    File : pointer.hpp
    Role : Low-level memory / data storage utilities

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <optional>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <shared_mutex>
#include <type_traits>

namespace fe::core {
    using handle_t = uint64_t; // can be replaced by uint32_t

    template <typename _Ty>
    concept storable_t =
        (!std::is_void_v<_Ty>) &&
        (!std::is_reference_v<_Ty>) &&
        (std::is_move_constructible_v<_Ty>) &&
        (!std::is_abstract_v<_Ty>) &&
        (!std::is_array_v<_Ty>);

    template <typename _Ty>
    class pointer {
    public:
        constexpr pointer(handle_t index, handle_t generation) noexcept
            : m_index(index), m_generation(generation) {}
        ~pointer() = default;

        constexpr pointer() noexcept                = default;
        pointer(const pointer&) noexcept            = default;
        pointer& operator=(const pointer&) noexcept = default;

        constexpr handle_t index() const noexcept { return m_index; }
        constexpr handle_t generation() const noexcept { return m_generation; }

        constexpr bool operator==(const pointer&) const noexcept = default;
        constexpr bool operator!=(const pointer&) const noexcept = default;

    private:
        handle_t m_index{ std::numeric_limits<handle_t>::max() };
        handle_t m_generation{ 0 };

        template <storable_t>
        friend class typed_pointer_storage;
    };

    template <storable_t _Ty>
    class typed_pointer_storage {
    public:
        using pointer_t = pointer<_Ty>;

        typed_pointer_storage()  = default;
        ~typed_pointer_storage() = default;

        pointer_t create(_Ty value) {
            std::unique_lock lock(m_mutex);

            handle_t index{};
            if (!m_free_list.empty()) {
                index = m_free_list.back();
                m_free_list.pop_back();

                m_slots[index].object = std::move(value);
                m_slots[index].alive  = true;
                m_slots[index].generation++;
            }
            else {
                index = static_cast<handle_t>(m_slots.size());

                m_slots.emplace_back(_MySlot{ std::move(value) });
            }
            return pointer_t(index, m_slots[index].generation);
        }

        pointer_t create()
            requires std::default_initializable<_Ty>
        {
            return create(_Ty{});
        }

        void destroy(pointer_t handle) {
            std::unique_lock lock(m_mutex);
            if (!is_valid_locked(handle)) return;

            m_slots[handle.m_index].object.~_Ty();
            m_slots[handle.m_index].alive = false;

            m_free_list.push_back(handle.m_index);
        }

        _Ty* get(pointer_t handle) noexcept {
            std::shared_lock lock(m_mutex);
            if (!is_valid_locked(handle)) return nullptr;
            return std::addressof(m_slots[handle.m_index].object);
        }

        const _Ty* get(pointer_t handle) const noexcept {
            std::shared_lock lock(m_mutex);
            if (!is_valid_locked(handle)) return nullptr;
            return std::addressof(m_slots[handle.m_index].object);
        }

        bool is_valid(pointer_t handle) const noexcept {
            std::shared_lock lock(m_mutex);
            return is_valid_locked(handle);
        }

        size_t live_count() const noexcept {
            std::shared_lock lock(m_mutex);
            return m_slots.size() - m_free_list.size();
        }

    private:
        bool is_valid_locked(pointer_t handle) const noexcept {
            if (handle.m_index >= m_slots.size()) return false;
            if (!m_slots[handle.m_index].alive) return false;
            return m_slots[handle.m_index].generation == handle.m_generation;
        }

        struct _MySlot {
            _Ty      object;
            handle_t generation;
            bool     alive;

            _MySlot(_Ty&& value, handle_t generation = 0, bool alive = true)
                : object(std::move(value)), generation(generation), alive(alive) {}

            _MySlot(const _Ty& value, handle_t generation = 0, bool alive = true)
                : object(value), generation(generation), alive(alive) {}
        };

        std::vector<_MySlot>  m_slots;
        std::vector<handle_t> m_free_list;

        mutable std::shared_mutex m_mutex;
    };

    struct base_storage {
        virtual ~base_storage() = default;
    };

    template <storable_t _Ty>
    struct derived_storage : base_storage {
        typed_pointer_storage<_Ty> storage;
    };

    class pointer_storage {
    public:
        pointer_storage()  = default;
        ~pointer_storage() = default;

        template <storable_t _Ty>
        typed_pointer_storage<_Ty>& get_storage() {
            std::unique_lock lock(m_registry_mutex);

            std::type_index id = std::type_index(typeid(_Ty));
            auto            it = m_storages.find(id);
            if (it != m_storages.end()) {
                return static_cast<derived_storage<_Ty>*>(it->second.get())->storage;
            }

            auto  up  = std::make_unique<derived_storage<_Ty>>();
            auto* ptr = &up->storage;
            m_storages.emplace(id, std::move(up));
            return *ptr;
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<base_storage>> m_storages;

        mutable std::shared_mutex m_registry_mutex;
    };

} // namespace fe::core
