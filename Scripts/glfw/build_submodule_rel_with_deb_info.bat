::===============================================
::
::  Forr Engine - Scripts
::
::  File : build_submodule_rel_with_deb_info.bat
::  Role : builds GLFW in release with debug info mode
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\glfw"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
cmake --build build --config RelWithDebInfo

popd