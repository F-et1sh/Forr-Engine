/*===============================================

    Forr Engine

    File : pointer.hpp
    Role : slot map

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <array>
#include <cstddef>
#include <concepts>
#include <functional>
#include <limits>
#include <new>
#include <tuple>
#include <utility>

#include "attributes.hpp"

namespace fe {
    using default_handle_t     = uint32_t;
    using default_generation_t = uint32_t;
    using default_packed_t     = uint64_t;

    struct empty_custom_fields_t {};

    template <typename T>
    concept storable_t =
        !std::is_void_v<T> &&
        !std::is_reference_v<T> &&
        !std::is_const_v<T> &&
        !std::is_volatile_v<T> &&
        !std::is_array_v<T> &&
        !std::is_function_v<T> &&
        std::destructible<T>;

    template <typename T>
    concept pointer_t = requires(T t) {
        typename T::HandleType;
        typename T::GenerationType;

        { T{} };

        { T(std::declval<typename T::HandleType>(),
            std::declval<typename T::GenerationType>()) };

        { t.index() } -> std::same_as<typename T::HandleType>;
        { t.generation() } -> std::same_as<typename T::GenerationType>;
    };

    template <typename T,
              typename HandleT,
              typename GenerationT,
              typename CustomFields,
              typename PackedT>
    concept pointer_packer_t = requires(HandleT      index,
                                        GenerationT  generation,
                                        CustomFields fields) {
        { T::operator()(index, generation, fields) } -> std::same_as<PackedT>;
    };

    template <typename T,
              typename HandleT,
              typename GenerationT,
              typename CustomFields,
              typename PackedT>
    concept pointer_unpacker_t = requires(PackedT packed) {
        { T::operator()(packed) } -> std::same_as<std::tuple<HandleT, GenerationT, CustomFields>>;
    };

    template <typename T,
              typename HandleT      = default_handle_t,
              typename GenerationT  = default_generation_t,
              typename PackedT      = default_packed_t,
              typename CustomFields = empty_custom_fields_t>
    class FORR_NODISCARD pointer {
    public:
        inline constexpr static PackedT PACKING_SHIFT = sizeof(GenerationT) * 8;

        static_assert(sizeof(HandleT) * 8 + sizeof(GenerationT) * 8 <= sizeof(PackedT) * 8,
                      "HandleT and GenerationT do not fit into PackedT");

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
                         static_cast<GenerationT>(packed & std::numeric_limits<GenerationT>::max()),
                         CustomFields{} };
            }
        };

        using HandleType     = HandleT;
        using GenerationType = GenerationT;

    public:
        constexpr pointer(HandleT index, GenerationT generation) noexcept
            : m_index(index), m_generation(generation) {}
        ~pointer() = default;

        template <typename UnpackFn = DefaultUnpacker>
            requires pointer_unpacker_t<UnpackFn, HandleT, GenerationT, CustomFields, PackedT>
        constexpr explicit pointer(PackedT packed) noexcept {
            auto unpacked = UnpackFn::operator()(packed);

            m_index         = std::get<0>(unpacked);
            m_generation    = std::get<1>(unpacked);
            m_custom_fields = std::get<2>(unpacked);
        }

        constexpr pointer() noexcept                = default;
        pointer(const pointer&) noexcept            = default;
        pointer& operator=(const pointer&) noexcept = default;

        FORR_NODISCARD constexpr HandleT     index() const noexcept { return m_index; }
        FORR_NODISCARD constexpr GenerationT generation() const noexcept { return m_generation; }

        FORR_NODISCARD CustomFields& custom_fields() noexcept
            requires(!std::is_same_v<CustomFields, empty_custom_fields_t>)
        {
            return m_custom_fields;
        }
        FORR_NODISCARD constexpr const CustomFields& custom_fields() const noexcept
            requires(!std::is_same_v<CustomFields, empty_custom_fields_t>)
        {
            return m_custom_fields;
        }

        template <typename PackFn = DefaultPacker>
            requires pointer_packer_t<PackFn, HandleT, GenerationT, CustomFields, PackedT>
        FORR_NODISCARD constexpr PackedT packed() const noexcept { return PackFn::operator()(m_index, m_generation, m_custom_fields); }

        template <typename PackFn = DefaultPacker>
            requires pointer_packer_t<PackFn, HandleT, GenerationT, CustomFields, PackedT>
        FORR_NODISCARD static constexpr PackedT packed(const pointer& pointer_to_pack) noexcept {
            return PackFn::operator()(pointer_to_pack.m_index,
                                      pointer_to_pack.m_generation,
                                      pointer_to_pack.m_custom_fields);
        }

        template <typename UnpackFn = DefaultUnpacker>
            requires pointer_unpacker_t<UnpackFn, HandleT, GenerationT, CustomFields, PackedT>
        FORR_NODISCARD static constexpr pointer from_packed(PackedT packed) noexcept {
            const auto unpacked = UnpackFn::operator()(packed);

            pointer result{};
            result.m_index         = std::get<0>(unpacked);
            result.m_generation    = std::get<1>(unpacked);
            result.m_custom_fields = std::get<2>(unpacked);

            return result;
        }

        FORR_NODISCARD constexpr bool is_valid() const noexcept {
            return m_index != std::numeric_limits<HandleT>::max() &&
                   m_generation != std::numeric_limits<GenerationT>::max();
        }

        FORR_NODISCARD constexpr bool operator==(const pointer& other) const noexcept {
            if constexpr (!std::is_same_v<CustomFields, empty_custom_fields_t>) {
                return m_index == other.m_index &&
                       m_generation == other.m_generation &&
                       m_custom_fields == other.m_custom_fields;
            }
            else {
                return m_index == other.m_index &&
                       m_generation == other.m_generation;
            }
        }
        FORR_NODISCARD constexpr bool operator!=(const pointer& other) const noexcept { return !(*this == other); }

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

        // TEMORARY
        FORR_CLASS_NONCOPYABLE(typed_pointer_storage)
        // TEMORARY
        FORR_CLASS_NONMOVABLE(typed_pointer_storage)

        FORR_NODISCARD PointerT create(const T& value) { return emplace(value); }
        FORR_NODISCARD PointerT create(T&& value) { return emplace(std::move(value)); }

        FORR_NODISCARD PointerT create()
            requires std::default_initializable<T>
        {
            return emplace();
        }

        template <typename... Args>
        FORR_NODISCARD PointerT emplace(Args&&... args) {
            typename PointerT::HandleType index{};

            if (!m_free_list.empty()) {
                index = m_free_list.back();

                // if you're getting an error here, then most likely you are :
                // - passing wrong arguments to the constructor ( or wrong number of arguments; zero counts )
                // - forgot 'std::move()' for movable-only objects
                std::construct_at(get_ptr(index), std::forward<Args>(args)...);

                m_free_list.pop_back();
                m_slots_alive[index] = true;
                m_slots_generation[index]++;
            }
            else {
                if (m_slots_generation.size() >= std::numeric_limits<typename PointerT::HandleType>::max()) {
                    fe::logging::fatal("Handle overflow");
                }

                index = static_cast<typename PointerT::HandleType>(m_slots_generation.size());

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
            return get_ptr(handle.index());
        }

        FORR_NODISCARD const T* get(PointerT handle) const {
            if (!is_valid(handle)) return nullptr;
            return get_ptr(handle.index());
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
            return std::launder(reinterpret_cast<T*>(m_slots_object[index].storage.data()));
        }

        FORR_NODISCARD const T* get_ptr(size_t index) const noexcept {
            return std::launder(reinterpret_cast<const T*>(m_slots_object[index].storage.data()));
        }

        // devided to be more cache friendly
        std::vector<Slot>                              m_slots_object;
        std::vector<typename PointerT::GenerationType> m_slots_generation;
        std::vector<uint8_t>                           m_slots_alive; // use 'uint8_t' instead of 'bool' for simple byte-addressable storage
        //

        std::vector<typename PointerT::HandleType> m_free_list;
    };

} // namespace fe

namespace std {
    template <typename T, typename HandleT, typename GenerationT, typename PackedT, typename CustomFields>
    struct hash<fe::pointer<T, HandleT, GenerationT, PackedT, CustomFields>> {
        constexpr std::size_t operator()(const fe::pointer<T, HandleT, GenerationT, PackedT, CustomFields>& p) const noexcept {
            return std::hash<PackedT>{}(p.packed());
        }
    };
} // namespace std
