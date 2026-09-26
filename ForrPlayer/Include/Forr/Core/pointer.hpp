/*===============================================

    Forr Engine

    File : pointer.hpp
    Role : slot map

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <type_traits>

#include "attributes.hpp"

namespace fe {
    using default_handle_t     = uint32_t;
    using default_generation_t = uint32_t;
    using default_packed_t     = uint64_t;

    struct empty_custom_fields_t {};

    template <typename T>
    concept storable_t =
        (!std::is_void_v<T>) &&
        (!std::is_reference_v<T>) &&
        (std::is_move_constructible_v<T>) &&
        (!std::is_abstract_v<T>) &&
        (!std::is_array_v<T>);

    template <typename T,
              typename HandleT,
              typename GenerationT>
    concept pointer_t = requires(T t) {
        typename T::HandleT;
        typename T::GenerationT;

        { T() };
        { T(std::declval<typename T::HandleT>(), std::declval<typename T::GenerationT>()) };

        { t.index() } -> std::same_as<typename T::HandleT>;
        { t.generation() } -> std::same_as<typename T::GenerationT>;
    };

    template <typename T,
              typename HandleT,
              typename GenerationT,
              typename PackedT,
              typename CustomFields>
    concept pointer_packer_t = requires(T t) {
        { t(std::declval<HandleT>(),
            std::declval<GenerationT>(),
            std::declval<PackedT>(),
            std::declval<CustomFields>()) } -> std::same_as<PackedT>;
    };

    template <typename T,
              typename HandleT      = default_handle_t,
              typename GenerationT  = default_generation_t,
              typename PackedT      = default_packed_t,
              typename CustomFields = empty_custom_fields_t>
    class FORR_NODISCARD pointer {
    public:
        inline constexpr static PackedT PACKING_SHIFT = sizeof(GenerationT) * 8;

        struct DefaultPacker {
        public:
            FORR_NODISCARD static constexpr PackedT operator()(HandleT index, GenerationT generation, CustomFields custom_fields) noexcept {
                return (static_cast<PackedT>(index) << PACKING_SHIFT) | static_cast<PackedT>(generation);
            }
        };

        struct DefaultUnpacker {
        public:
            FORR_NODISCARD static constexpr std::tuple<HandleT, GenerationT, CustomFields> operator()(PackedT packed) noexcept {
                return { static_cast<HandleT>(packed >> PACKING_SHIFT),
                         static_cast<GenerationT>(packed & std::numeric_limits<GenerationT>::max()) };
            }
        };

        using HandleT     = HandleT;
        using GenerationT = GenerationT;

    public:
        constexpr pointer(HandleT index, GenerationT generation) noexcept
            : m_index(index), m_generation(generation) {}
        ~pointer() = default;

        template <pointer_packer_t UnpackFn = DefaultUnpacker>
        constexpr explicit pointer(PackedT packed) noexcept {
            auto unpacked   = UnpackFn::operator()(packed);
            m_index         = std::get<0>(unpacked);
            m_generation    = std::get<1>(unpacked);
            m_custom_fields = std::get<2>(unpacked);
        }

        constexpr pointer() noexcept                = default;
        pointer(const pointer&) noexcept            = default;
        pointer& operator=(const pointer&) noexcept = default;

        FORR_NODISCARD constexpr HandleT     index() const noexcept { return m_index; }
        FORR_NODISCARD constexpr GenerationT generation() const noexcept { return m_generation; }

        FORR_NODISCARD constexpr CustomFields& custom_fields() noexcept
            requires(!std::is_same_v<CustomFields, empty_custom_fields_t>)
        {
            return m_custom_fields;
        }
        FORR_NODISCARD constexpr const CustomFields& custom_fields() noexcept const
            requires(!std::is_same_v<CustomFields, empty_custom_fields_t>)
        {
            return m_custom_fields;
        }

        template <pointer_packer_t PackFn = DefaultPacker>
        FORR_NODISCARD constexpr PackedT packed() const noexcept { return PackFn::operator()(m_index, m_generation, m_custom_fields); }

        template <pointer_packer_t UnpackFn = DefaultUnpacker>
        FORR_NODISCARD static constexpr pointer from_packed(PackedT packed) noexcept {
            return pointer::pointer<UnpackFn>(packed);
        }

        FORR_NODISCARD constexpr bool is_valid() const noexcept {
            return m_index != std::numeric_limits<HandleT>::max() &&
                   m_generation != std::numeric_limits<GenerationT>::max();
        }

        FORR_NODISCARD constexpr bool operator==(const pointer&) const noexcept = default;
        FORR_NODISCARD constexpr bool operator!=(const pointer&) const noexcept = default;

        FORR_NODISCARD constexpr operator bool() const noexcept { return this->is_valid(); }

    private:
        HandleT                             m_index{ std::numeric_limits<HandleT>::max() };
        GenerationT                         m_generation{ std::numeric_limits<GenerationT>::max() };
        FORR_NO_UNIQUE_ADDRESS CustomFields m_custom_fields{};
    };

    template <typename T,
              typename HandleT     = default_handle_t,
              typename GenerationT = default_generation_t,
              typename PackedT     = default_packed_t,
              typename PackFn      = pointer<T, HandleT, GenerationT, PackedT>::DefaultPacker,
              typename UnpackFn    = pointer<T, HandleT, GenerationT, PackedT>::DefaultUnpacker>
    struct pointer_hash {
        constexpr std::size_t operator()(const pointer<T, HandleT, GenerationT, PackedT>& p) const noexcept {
            return std::hash<PackedT>{}(p.packed<PackFn>());
        }
    };

    template <storable_t T, pointer_t PointerT = pointer<T>>
    class typed_pointer_storage {
    public:
        typed_pointer_storage() = default;
        ~typed_pointer_storage() {
            for (size_t i = 0; i < m_slots_object.size(); i++) {
                if (m_slots_alive[i]) {
                    std::destroy_at(get_ptr(i));
                }
            }
        }

        FORR_NODISCARD PointerT create(const T& value) { return emplace(value); }
        FORR_NODISCARD PointerT create(T&& value) { return emplace(std::move(value)); }

        FORR_NODISCARD PointerT create()
            requires std::default_initializable<T>
        {
            return emplace();
        }

        template <typename... Args>
        FORR_NODISCARD PointerT emplace(Args&&... args) {
            PointerT::HandleT index{};

            if (!m_free_list.empty()) {
                index = m_free_list.back();
                m_free_list.pop_back();

                std::construct_at(get_ptr(index), std::forward<Args>(args)...);
                m_slots_alive[index] = true;
                m_slots_generation[index]++;
            }
            else {
                index = static_cast<PointerT::HandleT>(m_slots_generation.size());

                if (m_slots_generation[index] == std::numeric_limits<PointerT::GenerationT>::max() - 1) {
                    fe::logging::fatal("Generation overflow in fe::typed_pointer_storage::emplace()\nPointer's generation index reached %s",
                                       std::to_string(std::numeric_limits<PointerT::GenerationT>::max() - 1).c_str());
                }

                m_slots_object.emplace_back();
                m_slots_generation.emplace_back(0);
                m_slots_alive.emplace_back(false);

                try {
                    std::construct_at(get_ptr(index), std::forward<Args>(args)...);
                    m_slots_alive[index] = true;
                }
                catch (...) {
                    m_slots_alive.pop_back();
                    m_slots_generation.pop_back();
                    m_slots_object.pop_back();

                    fe::logging::fatal("Failed to create an object in fe::typed_pointer_storage::emplace()");
                }
            }

            return PointerT(index, m_slots_generation[index]);
        }

        void destroy(PointerT handle) {
            if (!is_valid(handle)) return;

            std::destroy_at(get_ptr(handle.index()));
            m_slots_alive[handle.index()] = false;
            m_free_list.emplace_back(handle.index());
        }

        FORR_NODISCARD T* get(PointerT handle) {
            if (!is_valid(handle)) return nullptr;
            return *get_ptr(handle.index());
        }

        FORR_NODISCARD const T* get(PointerT handle) const {
            if (!is_valid(handle)) return nullptr;
            return *get_ptr(handle.index());
        }

        FORR_NODISCARD bool is_valid(PointerT handle) const {
            if (handle.index() >= m_slots_alive.size()) return false;
            if (!m_slots_alive[handle.index()]) return false;
            return m_slots_generation[handle.index()] == handle.generation();
        }

        FORR_NODISCARD size_t live_count() const noexcept {
            return m_slots_alive.size() - m_free_list.size();
        }

        // this function runs your lambda through all objects of the storage.
        // it can be invoked by :
        // [](T&, fe::pointer<T>) -> void {}
        // [](fe::pointer<T>, T&) -> void {}
        // [](T&) -> void {}
        // [](fe::pointer<T>) -> void {}
        // [](const T&, fe::pointer<T>) -> void {}
        // [](fe::pointer<T>, const T&) -> void {}
        // [](const T&) -> void {}
        template <typename _Func>
        void for_each(_Func&& func) {
            for (size_t i = 0; i < m_slots_object.size(); i++) {
                if (!m_slots_alive[i]) continue;

                T&       object = *get_ptr(i);
                PointerT ptr(i, m_slots_generation[i]);

                if constexpr (std::is_invocable_v<_Func, T&, PointerT>) {
                    func(object, ptr);
                }
                else if constexpr (std::is_invocable_v<_Func, PointerT, T&>) {
                    func(ptr, object);
                }
                else if constexpr (std::is_invocable_v<_Func, T&>) {
                    func(object);
                }
                else if constexpr (std::is_invocable_v<_Func, PointerT>) {
                    func(ptr);
                }
                else {
                    static_assert(std::false_type::value, "fe::typed_pointer_storage : for_each lambda has invalid signature");
                }
            }
        }

        // this function runs your lambda through all objects of the storage
        // it can be invoked by :
        // [](fe::pointer<T>) -> void {}
        // [](const T&, fe::pointer<T>) -> void {}
        // [](fe::pointer<T>, const T&) -> void {}
        // [](const T&) -> void {}
        template <typename _Func>
        void for_each(_Func&& func) const {
            for (size_t i = 0; i < m_slots_object.size(); i++) {
                if (!m_slots_alive[i]) continue;

                const T& object = *get_ptr(i);
                PointerT ptr(i, m_slots_generation[i]);

                if constexpr (std::is_invocable_v<_Func, const T&, PointerT>) {
                    func(object, ptr);
                }
                else if constexpr (std::is_invocable_v<_Func, PointerT, const T&>) {
                    func(ptr, object);
                }
                else if constexpr (std::is_invocable_v<_Func, const T&>) {
                    func(object);
                }
                else if constexpr (std::is_invocable_v<_Func, PointerT>) {
                    func(ptr);
                }
                else {
                    static_assert(std::false_type::value, "fe::typed_pointer_storage : const for_each lambda has invalid signature");
                }
            }
        }

    private:
        struct Slot {
        public:
            alignas(T) std::array<std::byte, sizeof(T)> storage{};
        };

        FORR_NODISCARD T* get_ptr(size_t index) noexcept {
            return reinterpret_cast<T*>(std::addressof(m_slots_object[index].storage));
        }

        FORR_NODISCARD const T* get_ptr(size_t index) const noexcept {
            return reinterpret_cast<const T*>(std::addressof(m_slots_object[index].storage));
        }

        // devided to be more cache friendly
        std::vector<Slot>              m_slots_object;
        std::vector<PointerT::HandleT> m_slots_generation;
        std::vector<uint8_t>           m_slots_alive; // use 'uint8_t' instead of 'bool' for simple byte-addressable storage
        //

        std::vector<PointerT::HandleT> m_free_list;
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
