/*===============================================

    Forr Engine - Platform Module

    File : Window.hpp
    Role : window

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    class Window {
    public:
        Window();
        ~Window();

    private:
        struct Impl;
        Impl* p_Impl = nullptr;
    };
} // namespace fe
