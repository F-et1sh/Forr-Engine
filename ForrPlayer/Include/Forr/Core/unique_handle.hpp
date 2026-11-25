/*===============================================

    Forr Engine

    File : unique_handle.hpp
    Role : unified class to create RAII handles and/or scoped subscriptions

    Copyright (C) 2026 Farrah
    SPDX-License-Identifier: MIT

===============================================*/

#pragma once
#include <tuple>

#include "attributes.hpp"

namespace fe {
    template <typename DestroyFn, typename... Handles>
    class unique_handle {
    public:
        static constexpr std::size_t HANDLES_COUNT = sizeof...(Handles);

    public:
        unique_handle() = default;
        explicit unique_handle(Handles... handles) noexcept
            : m_handles(handles...) {}

        ~unique_handle() { this->free(); }

        FORR_CLASS_NONCOPYABLE(unique_handle)

        unique_handle(unique_handle&& other) noexcept
            : m_handles(std::move(other.m_handles)) {
            other.reset_handles();
        }

        unique_handle& operator=(unique_handle&& other) noexcept {
            if (this != &other) {
                this->free();
                m_handles = std::move(other.m_handles);
                other.reset_handles(); // NOT other.free()
            }
            return *this;
        }

        void free() noexcept {
            std::apply(DestroyFn{}, m_handles);
            this->reset_handles();
        }

        void attach(Handles... handles) noexcept {
            std::tuple<Handles...> new_handles{ handles... };

            if (m_handles != new_handles) {
                this->free();
                m_handles = std::move(new_handles);
            }
        }

        auto get() const noexcept { return get_target_handle(); }

        operator auto() const noexcept { return get_target_handle(); }

        template <typename T>
        auto get() const noexcept { return std::get<T>(m_handles); }

    private:
        void reset_handles() noexcept {
            m_handles = std::tuple<Handles...>{ Handles{}... };
        }

        auto get_target_handle() const noexcept {
            return std::get<HANDLES_COUNT - 1>(m_handles);
        }

    private:
        std::tuple<Handles...> m_handles{ Handles{}... };
    };

    namespace detail {
        template <typename... Args>
        struct unique_handle_traits_base {
            template <typename DestroyFn>
            using handle_type = unique_handle<DestroyFn, Args...>;
        };

        template <typename T>
        struct unique_handle_traits : unique_handle_traits<decltype(&T::operator())> {};

        template <typename... Args, bool Noexcept>
        struct unique_handle_traits<void (*)(Args...) noexcept(Noexcept)> : unique_handle_traits_base<Args...> {};
    } // namespace detail

    template <typename DestroyFn>
    using create_unique_handle = detail::unique_handle_traits<decltype(&DestroyFn::operator())>::template handle_type<DestroyFn>;

} // namespace fe
