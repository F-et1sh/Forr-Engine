/*===============================================

    Forr Engine

    File : pointer.hpp
    Role : 
    
    
    storage

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <shared_mutex>
#include <type_traits>

#include "attributes.hpp"

namespace fe {
    using handle_t = uint32_t; // can't be replaced by uint64_t

    template <typename _Ty>
    concept storable_t =
        (!std::is_void_v<_Ty>) &&
        (!std::is_reference_v<_Ty>) &&
        (std::is_move_constructible_v<_Ty>) &&
        (!std::is_abstract_v<_Ty>) &&
        (!std::is_array_v<_Ty>);

    template <typename _Ty>
    class FORR_NODISCARD pointer {
    public:
        constexpr pointer(handle_t index, handle_t generation) noexcept
            : m_index(index), m_generation(generation) {}
        ~pointer() = default;

        constexpr explicit pointer(uint64_t packed) noexcept { this->unpack(packed); }

        constexpr pointer() noexcept                = default;
        pointer(const pointer&) noexcept            = default;
        pointer& operator=(const pointer&) noexcept = default;

        constexpr handle_t index() const noexcept { return m_index; }
        constexpr handle_t generation() const noexcept { return m_generation; }

        constexpr uint64_t packed() const noexcept {
            return (static_cast<uint64_t>(m_index) << 32) | static_cast<uint64_t>(m_generation);
        }

        // the pointer is going to be changed after unpacking
        // it's not just getting you an unpacked pointer<_Ty>
        constexpr pointer<_Ty> unpack(uint64_t packed) noexcept {
            m_index      = static_cast<uint32_t>(packed >> 32);
            m_generation = static_cast<uint32_t>(packed & 0xFFFFFFFF);
            return *this;
        }

        constexpr bool operator==(const pointer&) const noexcept = default;
        constexpr bool operator!=(const pointer&) const noexcept = default;

        constexpr bool operator!() const noexcept {
            return m_index == std::numeric_limits<handle_t>::max() ||
                   m_generation == std::numeric_limits<handle_t>::max();
        }

    private:
        handle_t m_index{ std::numeric_limits<handle_t>::max() };
        handle_t m_generation{ std::numeric_limits<handle_t>::max() };

        template <storable_t>
        friend class typed_pointer_storage;
    };

    template <typename T>
    struct pointer_hash {
        constexpr std::size_t operator()(const pointer<T>& p) const noexcept {
            return std::hash<uint64_t>{}(p.packed());
        }
    };

    template <typename T>
    struct pointer_equal {
        constexpr bool operator()(const pointer<T>& a, const pointer<T>& b) const noexcept {
            return a == b;
        }
    };

    template <storable_t _Ty>
    class typed_pointer_storage {
    public:
        using pointer_t = pointer<_Ty>;

        typed_pointer_storage()  = default;
        ~typed_pointer_storage() = default;

        FORR_NODISCARD pointer_t create(const _Ty& value) {
            //std::unique_lock lock(m_mutex);

            handle_t index{};
            if (!m_free_list.empty()) {
                index = m_free_list.back();
                m_free_list.pop_back();

                m_slots_object[index] = value;
                m_slots_alive[index]  = true;
                m_slots_generation[index]++;
            }
            else {
                index = static_cast<handle_t>(m_slots_generation.size());

                m_slots_object.emplace_back(value);
                m_slots_generation.emplace_back(0);
                m_slots_alive.emplace_back(true);
            }
            return pointer_t(index, m_slots_generation[index]);
        }

        FORR_NODISCARD pointer_t create(_Ty&& value) {
            //std::unique_lock lock(m_mutex);

            handle_t index{};
            if (!m_free_list.empty()) {
                index = m_free_list.back();
                m_free_list.pop_back();

                m_slots_object[index] = std::move(value);
                m_slots_alive[index]  = true;
                m_slots_generation[index]++;
            }
            else {
                index = static_cast<handle_t>(m_slots_generation.size());

                m_slots_object.emplace_back(std::move(value));
                m_slots_generation.emplace_back(0);
                m_slots_alive.emplace_back(true);
            }
            return pointer_t(index, m_slots_generation[index]);
        }

        FORR_NODISCARD pointer_t create()
            requires std::default_initializable<_Ty>
        {
            return create(_Ty{});
        }

        void destroy(pointer_t handle) {
            //std::unique_lock lock(m_mutex);
            if (!is_valid_locked(handle)) return;

            m_slots_object[handle.m_index].~_Ty();
            m_slots_alive[handle.m_index] = false;

            m_free_list.push_back(handle.m_index);
        }

        FORR_NODISCARD _Ty* get(pointer_t handle) noexcept {
            //std::shared_lock lock(m_mutex);
            if (!is_valid_locked(handle)) return nullptr;
            return std::addressof(m_slots_object[handle.m_index]);
        }

        FORR_NODISCARD const _Ty* get(pointer_t handle) const noexcept {
            //std::shared_lock lock(m_mutex);
            if (!is_valid_locked(handle)) return nullptr;
            return std::addressof(m_slots_object[handle.m_index]);
        }

        FORR_NODISCARD bool is_valid(pointer_t handle) const noexcept {
            //std::shared_lock lock(m_mutex);
            return is_valid_locked(handle);
        }

        FORR_NODISCARD size_t live_count() const noexcept {
            //std::shared_lock lock(m_mutex);
            return m_slots_alive.size() - m_free_list.size();
        }

        // this function runs your lambda through all objects of the storage.
        // it can be invoked by :
        // [](_Ty&, fe::pointer<_Ty>) -> void {}
        // [](fe::pointer<_Ty>, _Ty&) -> void {}
        // [](_Ty&) -> void {}
        // [](fe::pointer<_Ty>) -> void {}
        // [](const _Ty&, fe::pointer<_Ty>) -> void {}
        // [](fe::pointer<_Ty>, const _Ty&) -> void {}
        // [](const _Ty&) -> void {}
        template <typename _Func>
        void for_each(_Func&& func) {
            //std::shared_lock lock(m_mutex);
            for (size_t i = 0; i < m_slots_object.size(); i++) {
                if (!m_slots_alive[i]) continue;

                if constexpr (std::is_invocable_v<_Func, _Ty&, pointer_t>) {
                    func(m_slots_object[i], pointer_t(i, m_slots_generation[i]));
                }
                else if constexpr (std::is_invocable_v<_Func, pointer_t, _Ty&>) {
                    func(pointer_t(i, m_slots_generation[i]), m_slots_object[i]);
                }
                else if constexpr (std::is_invocable_v<_Func, _Ty&>) {
                    func(m_slots_object[i]);
                }
                else if constexpr (std::is_invocable_v<_Func, pointer_t>) {
                    func(pointer_t(i, m_slots_generation[i]));
                }
                else {
                    static_assert(false, "fe::typed_pointer_storage : for_each lambda has invalid signature");
                }
            }
        }

        // this function runs your lambda through all objects of the storage
        // it can be invoked by :
        // [](fe::pointer<_Ty>) -> void {}
        // [](const _Ty&, fe::pointer<_Ty>) -> void {}
        // [](fe::pointer<_Ty>, const _Ty&) -> void {}
        // [](const _Ty&) -> void {}
        template <typename _Func>
        void for_each(_Func&& func) const {
            //std::shared_lock lock(m_mutex);
            for (size_t i = 0; i < m_slots_object.size(); i++) {
                if (!m_slots_alive[i]) continue;

                if constexpr (std::is_invocable_v<_Func, const _Ty&, pointer_t>) {
                    func(m_slots_object[i], pointer_t(i, m_slots_generation[i]));
                }
                else if constexpr (std::is_invocable_v<_Func, pointer_t, const _Ty&>) {
                    func(pointer_t(i, m_slots_generation[i]), m_slots_object[i]);
                }
                else if constexpr (std::is_invocable_v<_Func, const _Ty&>) {
                    func(m_slots_object[i]);
                }
                else if constexpr (std::is_invocable_v<_Func, pointer_t>) {
                    func(pointer_t(i, m_slots_generation[i]));
                }
                else {
                    static_assert(false, "fe::typed_pointer_storage : const for_each lambda has invalid signature");
                }
            }
        }

    private:
        FORR_NODISCARD bool is_valid_locked(pointer_t handle) const noexcept { // this needed for mutex's work
            if (handle.m_index >= m_slots_alive.size()) return false;
            if (!m_slots_alive[handle.m_index]) return false;
            return m_slots_generation[handle.m_index] == handle.m_generation;
        }

        // devided to be more cache friendly
        std::vector<_Ty>      m_slots_object;
        std::vector<handle_t> m_slots_generation;
        std::vector<bool>     m_slots_alive;
        //

        std::vector<handle_t> m_free_list;

        //mutable std::shared_mutex m_mutex; // this is removed for now
    };

    struct base_storage {
        virtual ~base_storage() = default;
    };

    template <storable_t _Ty>
    struct derived_storage : base_storage {
        typed_pointer_storage<_Ty> storage;
    };

    // this class might be slow. better - do not use it
    class pointer_storage {
    public:
        pointer_storage()  = default;
        ~pointer_storage() = default;

        template <storable_t _Ty>
        FORR_NODISCARD typed_pointer_storage<_Ty>& get_storage() {
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

} // namespace fe

namespace std {
    template <fe::storable_t T>
    struct std::hash<fe::pointer<T>> {
        constexpr std::size_t operator()(const fe::pointer<T>& p) const noexcept {
            return std::hash<uint64_t>{}(p.packed());
        }
    };

} // namespace std
