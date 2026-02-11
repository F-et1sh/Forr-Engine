/*===============================================

    Forr Engine

    File : Layer.hpp
    Role : Application layer interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "core/attributes.hpp"

namespace fe {
    class FORR_API ILayer {
    public:
        ILayer()          = default;
        virtual ~ILayer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}

        //virtual void OnEvent(Event& event) {}

        virtual void OnUpdate(float delta_time) {}
        virtual void OnRender() {}

    private:
    };
} // namespace fe
