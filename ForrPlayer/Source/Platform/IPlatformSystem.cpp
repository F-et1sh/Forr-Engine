/*===============================================

    Forr Engine

    File : IPlatformSystem.cpp
    Role : Platform system interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#include "pch.hpp"
#include "Platform/IPlatformSystem.hpp"

#include "PlatformSystemGLFW.hpp"

std::unique_ptr<fe::IPlatformSystem> fe::IPlatformSystem::Create(const PlatformSystemDesc& desc) {
    std::unique_ptr<fe::IPlatformSystem> result{};

    switch (desc.backend) {
        case PlatformBackend::GLFW:
            result = std::make_unique<PlatformSystemGLFW>();
            break;
        default:
            fe::logging::warning("The selected platform backend %i was not found. Using the default one", desc.backend);

            result = std::make_unique<PlatformSystemGLFW>();
            break;
    }

    result->Initialize(desc);

    return std::move(result);
}
