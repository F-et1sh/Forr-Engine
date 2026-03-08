::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy tinygltf include and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\tinygltf\\json.hpp" "..\\..\\ThirdParty\\tinygltf\\include\\" >nul
xcopy /y /i /s "..\\..\\External\\tinygltf\\tiny_gltf.h" "..\\..\\ThirdParty\\tinygltf\\include\\" >nul

copy "..\\..\\External\\tinygltf\\LICENSE" "..\\..\\ThirdParty\\tinygltf\\TINYGLTF_LICENSE.txt" >nul

:: there was a bug
rmdir /s /q "..\\..\\ThirdParty\\tinygltf\\include\\examples"

mkdir "..\\..\\ThirdParty\\tinygltf\\src\\"

(
echo #define TINYGLTF_IMPLEMENTATION

echo #define TINYGLTF_NO_STB_IMAGE
echo #define TINYGLTF_NO_STB_IMAGE_WRITE

echo // #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

echo // #define TINYGLTF_NO_INCLUDE_JSON // TODO : enable this when include JSON to the project

echo #include "stb_image.h"
echo #include "stb_image_write.h"

echo #include "tiny_gltf.h"

) > "..\\..\\ThirdParty\\tinygltf\\src\\tiny_gltf.cpp"