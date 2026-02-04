/*===============================================

    Forr Engine - Platform Module

    File : Window.hpp
    Role : window

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
    struct EngineDesc {
        int window_width{};
        int window_height{};

        EngineDesc() = default;
        ~EngineDesc() = default;
    };

    class Engine {
    public:
        Engine() = default;
        ~Engine() = default;

        void Release();
        void Initialize();

    private:
        struct Impl;
        Impl* p_Impl = nullptr;
    };
}