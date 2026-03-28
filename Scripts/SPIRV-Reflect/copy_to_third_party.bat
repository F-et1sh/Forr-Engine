::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy SPIRV-Reflect include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\SPIRV-Reflect\\spirv_reflect.h" "..\\..\\ThirdParty\\SPIRV-Reflect\\include\\" >nul
xcopy /y /i /s "..\\..\\External\\SPIRV-Reflect\\spirv_reflect.c" "..\\..\\ThirdParty\\SPIRV-Reflect\\src\\" >nul