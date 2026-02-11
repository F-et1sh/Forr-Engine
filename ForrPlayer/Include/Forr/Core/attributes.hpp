/*===============================================

    Forr Engine

    File : attributes.hpp
    Role : useful defines

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#if __cplusplus >= 201703L // C++17
#define FORR_MAYBE_UNUSED [[maybe_unused]]
#else
#define FORR_MAYBE_UNUSED
#endif

#define FORR_NODISCARD [[nodiscard]]

#define FORR_CLASS_NONCOPYABLE(T)    \
    T(const T&)            = delete; \
    T& operator=(const T&) = delete;

#define FORR_CLASS_MOVABLE(T)             \
    T(T&&) noexcept            = default; \
    T& operator=(T&&) noexcept = default;
