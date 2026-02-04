::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy GLFW include, library and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\glfw\\include" "..\\..\\ThirdParty\\glfw\\include" >nul
xcopy /y /i /s "..\\..\\External\\glfw\\build\\src\\Debug" "..\\..\\ThirdParty\\glfw\\win64_debug" >nul
xcopy /y /i /s "..\\..\\External\\glfw\\build\\src\\Release" "..\\..\\ThirdParty\\glfw\\win64_release" >nul
xcopy /y /i /s "..\\..\\External\\glfw\\build\\src\\MinSizeRel" "..\\..\\ThirdParty\\glfw\\win64_min_size_rel" >nul
xcopy /y /i /s "..\\..\\External\\glfw\\build\\src\\RelWithDebInfo" "..\\..\\ThirdParty\\glfw\\win64_rel_with_deb_info" >nul

copy "..\\..\\External\\glfw\\LICENSE.md" "..\\..\\ThirdParty\\glfw\\GLFW_LICENSE.md" >nul
