/*===============================================

    Forr Engine

    File : PlatformSystemGLFW.hpp
    Role : GLFW platform system implementation

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include "Platform/IPlatformSystem.hpp"

namespace fe {
    class PlatformSystemGLFW : public IPlatformSystem {
    public:
        PlatformSystemGLFW()  = default;
        ~PlatformSystemGLFW() = default;

        void Release() override;
        void Initialize(const PlatformSystemDesc& desc) override;

    private:
    };
} // namespace fe
