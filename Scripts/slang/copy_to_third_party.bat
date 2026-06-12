::===============================================
::
::  Forr Engine - Scripts
::
::  File : copy_to_third_party.bat
::  Role : copy Slang include, library and license
::		to its folder in ThirdParty
::
::  Copyright (C) 2026 Farrakh
::  All Rights Reserved.
::
::===============================================

@echo off

xcopy /y /i /s "..\\..\\External\\slang\\include" "..\\..\\ThirdParty\\slang\\include" >nul
xcopy /y /i /s "..\\..\\External\\slang\\build\\Debug\\lib" "..\\..\\ThirdParty\\slang\\debug" >nul
xcopy /y /i /s "..\\..\\External\\slang\\build\\Release\\lib" "..\\..\\ThirdParty\\slang\\release" >nul

copy "..\\..\\External\\slang\\LICENSE" "..\\..\\ThirdParty\\slang\\SLANG_LICENSE" >nul
xcopy /y /i /s "..\\..\\External\\slang\\LICENSES" "..\\..\\ThirdParty\\slang\\SLANG_LICENSES" >nul
