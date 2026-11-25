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
        template <typename ClassType, typename... Args>
        struct unique_handle_traits_fn_base {
            using handle_type = unique_handle<ClassType, Args...>;
        };

        template <typename T>
        struct unique_handle_traits_fn;

        template <typename ClassType, typename ReturnType, typename... Args>
        struct unique_handle_traits_fn<ReturnType (ClassType::*)(Args...) const noexcept> : unique_handle_traits_fn_base<ClassType, Args...> {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct unique_handle_traits_fn<ReturnType (ClassType::*)(Args...) const> : unique_handle_traits_fn_base<ClassType, Args...> {};
    } // namespace detail

    // TODO : use 'static operator()' when IntelliSense will be able to parse it
    //
    // If you get error C2938 - 'fe::create_unique_handle' : Failed to specialize alias template :
    // destroy structure's operator() must have the same signature as in 'ReturnType (ClassType::*)(Args...) const noexcept' or 'ReturnType (ClassType::*)(Args...) const'
    // don't forget to put 'const noexcept' or just 'const'
    template <typename DestroyFn>
    using create_unique_handle = detail::unique_handle_traits_fn<decltype(&DestroyFn::operator())>::handle_type;

} // namespace fe
