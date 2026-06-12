::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_dlls_to_build_folder_debug.bat
::  Role : copy Slang dlls to build folder of the engine
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

set OUTPUT_DIR=%~1
set SOLUTION_DIR=%~2
set SLANG_BIN=%SOLUTION_DIR%\\External\\slang\\build\\Debug\\bin

copy /y "%SLANG_BIN%\\slang.dll" "%OUTPUT_DIR%" >nul
copy /y "%SLANG_BIN%\\slang-compiler.dll" "%OUTPUT_DIR%" >nul