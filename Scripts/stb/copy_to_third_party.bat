::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy STB Image include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\stb\\stb_image.h" "..\\..\\ThirdParty\\stb\\include\\" >nul
copy "..\\..\\External\\stb\\LICENSE" "..\\..\\ThirdParty\\stb\\STB_LICENSE_MIT.txt" >nul

mkdir "..\\..\\ThirdParty\\stb\\src\\"

(
echo #define STB_IMAGE_IMPLEMENTATION
echo #include "stb_image.h"
) > "..\\..\\ThirdParty\\stb\\src\\stb_image.cpp"