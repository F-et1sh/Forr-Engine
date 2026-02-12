/*===============================================

    Forr Engine

    File : IPlatformSystem.hpp
    Role : Platform system interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once
#include <memory>
#include "core/attributes.hpp"

namespace fe {
    enum class FORR_API PlatformBackend {
        GLFW
    };

    struct FORR_API PlatformSystemDesc {
        PlatformBackend backend;
    };

    class FORR_API IPlatformSystem {
    public:
        virtual ~IPlatformSystem() = default;

        static std::unique_ptr<IPlatformSystem> Create(const PlatformSystemDesc& desc);

    protected:
        virtual void Release() {}
        virtual void Initialize(const PlatformSystemDesc& desc) {}
    };
} // namespace fe
