/*===============================================

    Forr Engine

    File : utility.hpp
    Role : classes to inherit - NonCopyable, Movable

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "attributes.hpp"

namespace fe::utility {
    class FORR_API NonCopyable {
    protected:
        constexpr NonCopyable() = default;
        ~NonCopyable()          = default;

    public:
        NonCopyable(const NonCopyable&)            = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };

    class FORR_API MovableOnly {
    protected:
        constexpr MovableOnly() = default;
        ~MovableOnly()          = default;

    public:
        MovableOnly(const MovableOnly&)            = delete;
        MovableOnly& operator=(const MovableOnly&) = delete;

        MovableOnly(MovableOnly&&) noexcept            = default;
        MovableOnly& operator=(MovableOnly&&) noexcept = default;
    };

} // namespace fe::utility
