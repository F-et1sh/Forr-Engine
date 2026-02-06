/*===============================================

    Forr Engine - Shared Module

    File : PlatformService.hpp
    Role : Platform service and interface

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe::shared {
    class PlatformService {
    public:
        PlatformService() = default;
        ~PlatformService() = default;

    private:
        void* p_NativeWindow = nullptr;
    };
}