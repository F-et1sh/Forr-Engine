::===============================================
::
::  Forr Engine - Scripts
::
::  File : build_submodule_debug.bat
::  Role : builds GLFW in debug mode
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\glfw"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
cmake --build build --config Debug

popd