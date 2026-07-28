::===============================================
::
::  Forr Engine - Scripts
::
::  File : build_submodule_release.bat
::  Role : builds Slang in release mode
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\slang"

cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release

popd
