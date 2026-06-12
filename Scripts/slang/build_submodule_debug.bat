::===============================================
::
::  Forr Engine - Scripts
::
::  File : build_submodule_debug.bat
::  Role : builds Slang in debug mode
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\slang"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug

popd