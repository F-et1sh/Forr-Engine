::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy Vulkan Memory Allocator include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\VulkanMemoryAllocator\\include\\vk_mem_alloc.h" "..\\..\\ThirdParty\\VulkanMemoryAllocator\\include\\" >nul
copy "..\\..\\External\\VulkanMemoryAllocator\\LICENSE.txt" "..\\..\\ThirdParty\\VulkanMemoryAllocator\\VMA_LICENSE.txt" >nul

mkdir "..\\..\\ThirdParty\\VulkanMemoryAllocator\\src"

(
echo #ifdef _WIN32
echo	#define VK_USE_PLATFORM_WIN32_KHR
echo #endif
echo #define VMA_IMPLEMENTATION
echo #define VMA_STATIC_VULKAN_FUNCTIONS 0
echo #define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
echo #include "Volk/volk.h"
echo #include "../include/vk_mem_alloc.h"
) > "..\\..\\ThirdParty\\VulkanMemoryAllocator\\src\\vk_mem_alloc.cpp"