/*===============================================

    Forr Engine

    File : Tools.hpp
    Role : defines

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

#define GLFW_CHECK_RESULT(func)                                                                              \
    {                                                                                                        \
        int result = func;                                                                                   \
        if (result != GLFW_TRUE) {                                                                           \
            fe::logging::error("[GLFW] Failed to call %s in file %s at line %i", #func, __FILE__, __LINE__); \
        }                                                                                                    \
    }

#define VK_CHECK_RESULT(func)                                                                                                           \
    {                                                                                                                                   \
        VkResult result = func;                                                                                                         \
        if (result != VK_SUCCESS) {                                                                                                     \
            fe::logging::error("[VULKAN] Failed to call %s in file %s at line %i\nError : %i", #func, __FILE__, __LINE__, int(result)); \
        }                                                                                                                               \
    }

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000
