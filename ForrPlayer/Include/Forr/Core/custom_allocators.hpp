/*===============================================

    Forr Engine

    File : custom_allocators.hpp
    Role : Arena and Pool allocators

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#include <cassert>
#include <type_traits>
#include <memory>

#include "attributes.hpp"

namespace fe {
    static inline constexpr size_t align_up(size_t alignment, size_t offset) noexcept {
        return (offset + alignment - 1) & ~(alignment - 1);
    }

    struct FORR_API ArenaMarker {
        size_t offset{};
    };

    class FORR_API Arena { // mostly per-frame container
    public:
        explicit Arena(size_t capacity) : m_capacity(capacity) {
            m_buffer = static_cast<std::byte*>(::operator new(capacity, std::align_val_t{ alignof(std::max_align_t) }));
        }

        ~Arena() {
            if (m_buffer) ::operator delete(m_buffer, std::align_val_t{ alignof(std::max_align_t) });
            m_capacity = 0;
            m_offset   = 0;
        }

        FORR_CLASS_NONCOPYABLE(Arena)

        Arena(Arena&& other) noexcept : m_buffer(other.m_buffer), m_capacity(other.m_capacity), m_offset(other.m_offset) {
            other.m_buffer   = nullptr;
            other.m_capacity = 0;
            other.m_offset   = 0;
        }

        Arena& operator=(Arena&& other) noexcept {
            if (this != &other) {
                if (m_buffer) ::operator delete(m_buffer, std::align_val_t{ alignof(std::max_align_t) });

                m_buffer   = other.m_buffer;
                m_capacity = other.m_capacity;
                m_offset   = other.m_offset;

                other.m_buffer   = nullptr;
                other.m_capacity = 0;
                other.m_offset   = 0;
            }
            return *this;
        }

        FORR_NODISCARD std::byte* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
            assert(alignment > 0 && (alignment & (alignment - 1)) == 0); // check that it's a power of two

            size_t aligned_offset = fe::align_up(alignment, m_offset);

            if (aligned_offset + size > m_capacity) {
                return nullptr;
            }

            std::byte* ptr = m_buffer + aligned_offset;
            m_offset       = aligned_offset + size;
            return ptr;
        }

        template <typename T = std::byte>
        FORR_NODISCARD std::span<T> allocate_span(size_t byte_size, size_t alignment = alignof(std::max_align_t)) {
            std::byte* raw_ptr = allocate(byte_size, alignment);
            if (!raw_ptr) return {};

            return std::span<T>(reinterpret_cast<T*>(raw_ptr), byte_size / sizeof(T));
        }

        void reset() { m_offset = 0; }

        FORR_NODISCARD constexpr size_t get_used_memory() const noexcept { return m_offset; }
        FORR_NODISCARD constexpr size_t get_available_memory() const noexcept { return m_capacity - m_offset; }

        FORR_NODISCARD constexpr ArenaMarker save() const noexcept {
            return { m_offset };
        }

        constexpr void restore(ArenaMarker marker) {
            assert(marker.offset <= m_offset);
            m_offset = marker.offset;
        }

        void reinitialize(size_t capacity) noexcept {
            *this = Arena{ capacity };
        }

        FORR_NODISCARD constexpr std::byte* data() noexcept {
            return m_buffer;
        }

        FORR_NODISCARD constexpr const std::byte* data() const noexcept {
            return m_buffer;
        }

    private:
        std::byte* m_buffer   = nullptr;
        size_t     m_capacity = 0;
        size_t     m_offset   = 0;
    };

    template <typename _Ty>
    class FORR_API Pool {
        struct FreeNode {
            FreeNode* next = nullptr;
        };

    public:
        explicit Pool(size_t count) {
            static_assert(sizeof(_Ty) >= sizeof(FreeNode*), "Object too small for pool");

            m_capacity = count;
            m_buffer   = ::operator new(count * sizeof(_Ty), std::align_val_t{ alignof(_Ty) });

            for (size_t i = 0; i < count; i++) {
                auto* node  = reinterpret_cast<FreeNode*>(m_buffer + i * sizeof(_Ty));
                node->next  = m_free_list;
                m_free_list = node;
            }
        }

        ~Pool() { ::operator delete(m_buffer, std::align_val_t{ alignof(_Ty) }); }

        FORR_CLASS_NONCOPYABLE(Pool)

        FORR_NODISCARD _Ty* allocate() {
            if (!m_free_list) return nullptr;

            std::byte* ptr = m_free_list;
            m_free_list    = m_free_list->next;
            return static_cast<_Ty*>(ptr);
        }

        void deallocate(_Ty* ptr) {
            auto* node  = reinterpret_cast<FreeNode*>(ptr);
            node->next  = m_free_list;
            m_free_list = node;
        }

        // don't forget about Pool::destroy()
        template <typename... Args>
        FORR_NODISCARD _Ty* create(Args&&... args) {
            _Ty* ptr = allocate();
            std::construct_at(ptr, std::forward<Args>(args)...);
            return ptr;
        }

        void destroy(_Ty* ptr) {
            std::destroy_at(ptr);
            deallocate(ptr);
        }

    private:
        std::byte* m_buffer    = nullptr;
        size_t     m_capacity  = 0;
        FreeNode*  m_free_list = nullptr;
    };

} // namespace fe
