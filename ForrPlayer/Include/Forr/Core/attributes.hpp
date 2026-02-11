/*===============================================

    Forr Engine

    File : attributes.hpp
    Role : useful defines.
        Based on Godot. MIT License ( https://github.com/godotengine/godot )

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

#ifdef _WIN32
#ifdef FORRENGINE_EXPORTS
#define FORR_API __declspec(dllexport)
#else
#define FORR_API __declspec(dllimport)
#endif
#else
#define FORR_API
#endif

// Should always inline no matter what
#ifndef FORR_ALWAYS_INLINE
#if defined(__GNUC__)
#define FORR_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define FORR_ALWAYS_INLINE __forceinline
#else
#define FORR_ALWAYS_INLINE inline
#endif
#endif

// Should always inline, except in dev builds because it makes debugging harder,
// or `size_enabled` builds where inlining is actively avoided
#ifndef FORR_FORCE_INLINE
#if defined(_DEBUG) || defined(SIZE_EXTRA)
#define FORR_FORCE_INLINE inline
#else
#define FORR_FORCE_INLINE FORR_ALWAYS_INLINE
#endif
#endif

// Should never inline
#ifndef FORR_NO_INLINE
#if defined(__GNUC__)
#define FORR_NO_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define FORR_NO_INLINE __declspec(noinline)
#else
#define FORR_NO_INLINE
#endif
#endif

// In some cases [[nodiscard]] will get false positives,
// we can prevent the warning in specific cases by preceding the call with a cast.
#ifndef FORR_ALLOW_DISCARD
#define FORR_ALLOW_DISCARD (void)
#endif

// Windows badly defines a lot of stuff we'll never use. Undefine it.
#ifdef _WIN32
#undef min        // override standard definition
#undef max        // override standard definition
#undef ERROR      // override ( really stupid ) wingdi.h standard definition
#undef DELETE     // override ( another really stupid ) winnt.h standard definition
#undef MessageBox // override winuser.h standard definition
#undef Error
#undef OK
#undef CONNECT_DEFERRED // override from Windows SDK, clashes with Object enum
#undef MONO_FONT
#endif

#define FORR_CLASS_NONCOPYABLE(T)    \
    T(const T&)            = delete; \
    T& operator=(const T&) = delete;

#define FORR_CLASS_MOVABLE(T)             \
    T(T&&) noexcept            = default; \
    T& operator=(T&&) noexcept = default;
