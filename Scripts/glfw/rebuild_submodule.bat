::===============================================
::
::  Forr Engine - Scripts
::
::  File : rebuild_submodule.bat
::  Role : rebuild GLFW submodule
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

pushd "%~dp0\..\..\External\glfw"

rmdir /q /s build

popd

call build_submodule_debug.bat
call build_submodule_release.bat
call build_submodule_min_size_rel.bat