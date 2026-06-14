#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "Volk/volk.h"
#include "../include/vk_mem_alloc.h"
