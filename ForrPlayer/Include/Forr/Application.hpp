/*===============================================

    Forr Engine

    File : Application.hpp
    Role : main class

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include <vector>

#include "Layer.hpp"

namespace fe {
    struct FORR_API ApplicationDesc {
    public:
        int window_width  = 0;
        int window_height = 0;

        ApplicationDesc()  = default;
        ~ApplicationDesc() = default;
    };

    class FORR_API Application {
    public:
        Application() = default;
        Application(const ApplicationDesc& desc) { this->Initialize(desc); }

        ~Application() = default;

        void Release();
        void Initialize(const ApplicationDesc& desc);

        void Run();

        template<typename T> requires std::is_base_of_v<ILayer, T>
        inline void PushLayer() {
            //m_LayerStack.push_back(std::make_unique<T>());
        }

    private:
        //std::vector<std::unique_ptr<ILayer>> m_LayerStack;
    };
} // namespace fe
