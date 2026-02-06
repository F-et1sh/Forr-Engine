/*===============================================

    Forr Engine - Engine Module

    File : Common.hpp
    Role : FORR_API

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#ifdef _WIN32
#ifdef FORRENGINE_EXPORTS
#define FORR_API __declspec(dllexport)
#else
#define FORR_API __declspec(dllimport)
#endif
#else
#define FORR_API
#endif