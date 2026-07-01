/*===============================================

    Forr Engine

    File : Tools.hpp
    Role : helpful defines

    Copyright (C) 2026 Farrakh
    All Rights Reserved.

===============================================*/

#pragma once

namespace fe {
#define GLFW_CHECK_RESULT(func)                                                                              \
    {                                                                                                        \
        int result = func;                                                                                   \
        if (result != GLFW_TRUE) FORR_UNLIKELY {                                                             \
            fe::logging::error("[GLFW] Failed to call %s in file %s at line %i", #func, __FILE__, __LINE__); \
        }                                                                                                    \
    }

#define VK_CHECK_RESULT(func)                                                                                                           \
    {                                                                                                                                   \
        VkResult result = func;                                                                                                         \
        if (result != VK_SUCCESS) FORR_UNLIKELY {                                                                                       \
            fe::logging::error("[VULKAN] Failed to call %s in file %s at line %i\nError : %i", #func, __FILE__, __LINE__, int(result)); \
        }                                                                                                                               \
    }
} // namespace fe
