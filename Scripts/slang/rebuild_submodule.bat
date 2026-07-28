::===============================================
::
::  Forr Engine - Scripts
::
::  File : rebuild_submodule.bat
::  Role : rebuild Slang submodule
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\slang"

rmdir /q /s build

popd

call build_submodule_debug.bat
call build_submodule_release.bat
