/*===============================================

    Forr Engine - Core Module

    File : attributes.hpp
    Role : useful macros

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#ifdef _WIN32
    #ifdef FORRENGINE_EXPORTS
        #define FORR_API __declspec(dllexport)
    #else
        #define FORR_API __declspec(dllimport)
    #endif
#else
    #define FORR_API
#endif

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
