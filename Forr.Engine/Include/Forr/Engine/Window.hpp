/*===============================================

    Forr Engine - Engine Module

    File : Window.cpp
    Role : window

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include "Common/Common.hpp"

namespace fe::engine {
    struct FORR_API WindowDesc {
        int window_width  = 0;
        int window_height = 0;

        WindowDesc()  = default;
        ~WindowDesc() = default;
    };

    class FORR_API Window {
    public:
        Window();
        ~Window();

        void Release();
        void Initialize(const WindowDesc& desc);

    private:
        struct Impl;
        Impl* p_Impl;
    };
} // namespace fe::engine
